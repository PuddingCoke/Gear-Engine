#include <Gear/Core/D3D12Core/DescriptorHeap.h>

#include <Gear/Core/D3D12Core/ContiguousDescriptorHandle.h>

#include <Gear/Core/D3D12Core/UnorderedDescriptorHandle.h>

#include <algorithm>

namespace Gear::Core::D3D12Core
{
	DescriptorHeap::DescriptorHeap(const uint32_t numDescriptorsParam,
	                               const uint32_t numContiguousDescriptorsParam,
	                               const D3D12_DESCRIPTOR_HEAP_TYPE typeParam,
	                               const D3D12_DESCRIPTOR_HEAP_FLAGS flagsParam) :
		numDescriptors(numDescriptorsParam),
		numContiguousDescriptors(numContiguousDescriptorsParam),
		numUnorderedDescriptors(numDescriptorsParam - numContiguousDescriptorsParam),
		type(typeParam),
		incrementSize(GraphicsDevice::get()->GetDescriptorHandleIncrementSize(typeParam))
	{
		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.NodeMask = 0;
		desc.NumDescriptors = numDescriptors;
		desc.Type = type;
		desc.Flags = flagsParam;

		GraphicsDevice::get()->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&descriptorHeap));

		cpuHandleStart = descriptorHeap->GetCPUDescriptorHandleForHeapStart();

		if (flagsParam == D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE)
		{
			gpuHandleStart = descriptorHeap->GetGPUDescriptorHandleForHeapStart();
		}
		else
		{
			gpuHandleStart = CD3DX12_GPU_DESCRIPTOR_HANDLE();
		}

		//有序区初始为一整块空闲区间
		runs.push_back({ 0u, numContiguousDescriptors });

		unorderedIndices.reserve(numUnorderedDescriptors);

		for (uint32_t i = 0; i < numUnorderedDescriptors; i++)
		{
			unorderedIndices.push_back(i + numContiguousDescriptors);
		}
	}

	ID3D12DescriptorHeap* DescriptorHeap::get() const
	{
		return descriptorHeap.Get();
	}

	UniquePtr<DescriptorHandle> DescriptorHeap::allocContiguousDescriptor(const uint32_t num)
	{
		if (num == 0u)
		{
			THROWLOG(LOGERROR() << "连续描述符的申请数量不能为0");
		}

		std::lock_guard<std::mutex> lock(contiguousMutex);

		//首次适配，取出区间头部的一段
		for (size_t i = 0; i < runs.size(); i++)
		{
			Run& run = runs[i];

			if (run.count < num)
			{
				continue;
			}

			const uint32_t start = run.start;

			run.start += num;

			run.count -= num;

			if (run.count == 0u)
			{
				runs.erase(runs.begin() + i);
			}

			return makeUnique<ContiguousDescriptorHandle>(
				this, cpuHandleStart, gpuHandleStart, start, incrementSize, num);
		}

		uint32_t largestRun = 0u;

		for (const Run& run : runs)
		{
			largestRun = run.count > largestRun ? run.count : largestRun;
		}

		THROWLOG(LOGERROR() << "连续的描述符索引不足！需要" << num << "个，当前最大连续" << largestRun
		                    << "个");
	}

	UniquePtr<DescriptorHandle> DescriptorHeap::allocUnorderedDescriptor(const uint32_t num)
	{
		if (num == 0u)
		{
			THROWLOG(LOGERROR() << "无序描述符的申请数量不能为0");
		}

		std::lock_guard<std::mutex> lock(unorderedMutex);

		if (unorderedIndices.size() < num)
		{
			THROWLOG(LOGERROR() << "可用无序描述符索引不足！请先释放一些空闲资源！");
		}

		const std::vector<uint32_t> descriptorIndices(unorderedIndices.end() - num, unorderedIndices.end());

		unorderedIndices.resize(unorderedIndices.size() - num);

		return makeUnique<UnorderedDescriptorHandle>(
			this, cpuHandleStart, gpuHandleStart, incrementSize, descriptorIndices);
	}

	void DescriptorHeap::freeContiguousDescriptor(const uint32_t start, const uint32_t count)
	{
		if (count == 0u)
		{
			THROWLOG(LOGERROR() << "连续描述符的归还数量不能为0");
		}

		std::lock_guard<std::mutex> lock(contiguousMutex);

		const uint32_t end = start + count;

#ifdef _DEBUG
		if (end > numContiguousDescriptors)
		{
			THROWLOG(LOGERROR() << "归还的连续描述符区间越界");
		}
#endif // _DEBUG

		const auto it = std::lower_bound(runs.begin(),
		                                 runs.end(),
		                                 start,
		                                 [](const Run& run, const uint32_t value)
		{
			return run.start < value;
		});

#ifdef _DEBUG
		if (it != runs.end() && end > it->start)
		{
			THROWLOG(LOGERROR() << "归还的连续描述符区间与已有空闲区间重叠");
		}

		if (it != runs.begin() && (it - 1)->start + (it - 1)->count > start)
		{
			THROWLOG(LOGERROR() << "归还的连续描述符区间与已有空闲区间重叠");
		}
#endif // _DEBUG

		const auto inserted = runs.insert(it, Run{ start, count });

		//先与后一个空闲区间合并
		const auto next = inserted + 1;

		if (next != runs.end() && inserted->start + inserted->count == next->start)
		{
			inserted->count += next->count;

			runs.erase(next);
		}

		//再与前一个空闲区间合并
		if (inserted != runs.begin())
		{
			const auto prev = inserted - 1;

			if (prev->start + prev->count == inserted->start)
			{
				prev->count += inserted->count;

				runs.erase(inserted);
			}
		}
	}

	void DescriptorHeap::freeUnorderedDescriptor(const std::vector<uint32_t>& indices)
	{
		if (indices.empty())
		{
			THROWLOG(LOGERROR() << "无序描述符的归还数量不能为0");
		}

		std::lock_guard<std::mutex> lock(unorderedMutex);

		unorderedIndices.insert(unorderedIndices.end(), indices.begin(), indices.end());
	}

	uint32_t DescriptorHeap::getNumDescriptors() const
	{
		return numDescriptors;
	}

	uint32_t DescriptorHeap::getNumContiguousDescriptors() const
	{
		return numContiguousDescriptors;
	}

	uint32_t DescriptorHeap::getNumUnorderedDescriptors() const
	{
		return numUnorderedDescriptors;
	}
}
