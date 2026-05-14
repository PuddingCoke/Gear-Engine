#include<Gear/Core/D3D12Core/DescriptorHandle.h>

Gear::Core::D3D12Core::DescriptorHandle::DescriptorHandle() :
	cpuHandleStart{}, gpuHandleStart{}, currentOffset(), incrementSize(), baseIndex(), offsetLimit(), descriptorHeap(nullptr)
{
}

Gear::Core::D3D12Core::DescriptorHandle::DescriptorHandle(const CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandleStart, const CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandleStart,
	const uint32_t currentOffset, const uint32_t incrementSize, const uint32_t baseIndex, const uint32_t offsetLimit, const DescriptorHeap* descriptorHeap) :
	cpuHandleStart(cpuHandleStart), gpuHandleStart(gpuHandleStart), currentOffset(currentOffset), incrementSize(incrementSize), baseIndex(baseIndex), offsetLimit(offsetLimit), descriptorHeap(descriptorHeap)
{
}

CD3DX12_CPU_DESCRIPTOR_HANDLE Gear::Core::D3D12Core::DescriptorHandle::getCurrentCPUHandle() const
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(cpuHandleStart, getCurrentIndex(), incrementSize);
}

CD3DX12_GPU_DESCRIPTOR_HANDLE Gear::Core::D3D12Core::DescriptorHandle::getCurrentGPUHandle() const
{
	return CD3DX12_GPU_DESCRIPTOR_HANDLE(gpuHandleStart, getCurrentIndex(), incrementSize);
}

uint32_t Gear::Core::D3D12Core::DescriptorHandle::getCurrentIndex() const
{
	return baseIndex + currentOffset;
}

uint32_t Gear::Core::D3D12Core::DescriptorHandle::getIncrementSize() const
{
	return incrementSize;
}

uint32_t Gear::Core::D3D12Core::DescriptorHandle::getDistToEnd() const
{
	return offsetLimit - currentOffset;
}

const Gear::Core::D3D12Core::DescriptorHeap* Gear::Core::D3D12Core::DescriptorHandle::getDescriptorHeap() const
{
	return descriptorHeap;
}

void Gear::Core::D3D12Core::DescriptorHandle::move(const uint32_t num)
{
	currentOffset += num;

	currentOffset %= offsetLimit;
}

void Gear::Core::D3D12Core::DescriptorHandle::copyDescriptors(const uint32_t numCopy, DescriptorHandle copyDestHandle, DescriptorHandle copySrcHandle, const D3D12_DESCRIPTOR_HEAP_TYPE heapType)
{
	const uint32_t copyDestDistToEnd = copyDestHandle.getDistToEnd();

	const uint32_t copySrcDistToEnd = copySrcHandle.getDistToEnd();

	//拷贝目标有环绕
	if (copyDestDistToEnd < numCopy)
	{
		//拷贝源也有环绕
		if (copySrcDistToEnd < numCopy)
		{
			//环绕前的长度不一致
			if (copyDestDistToEnd > copySrcDistToEnd || copyDestDistToEnd < copySrcDistToEnd)
			{
				//这里要分三个部分进行拷贝

				//第一个拷贝部分的长度取两个环绕前长度最短的那个
				const uint32_t numCopyFirst = ((copyDestDistToEnd < copySrcDistToEnd) ? copyDestDistToEnd : copySrcDistToEnd);

				//第二个拷贝部分的长度取两个环绕前长度相减的绝对值
				const uint32_t numCopySecond = ((copyDestDistToEnd < copySrcDistToEnd) ? copySrcDistToEnd - copyDestDistToEnd : copyDestDistToEnd - copySrcDistToEnd);

				//第三个拷贝部分的长度为剩余拷贝长度
				const uint32_t numCopyThird = numCopy - numCopyFirst - numCopySecond;

				copyDescriptorsSimple(numCopyFirst, copyDestHandle, copySrcHandle, heapType);

				copyDescriptorsSimple(numCopySecond, copyDestHandle, copySrcHandle, heapType);

				copyDescriptorsSimple(numCopyThird, copyDestHandle, copySrcHandle, heapType);
			}
			//环绕前的长度一致
			else
			{
				//这里要分两个部分进行拷贝
				const uint32_t numCopyFirst = copyDestDistToEnd;

				const uint32_t numCopySecond = numCopy - numCopyFirst;

				copyDescriptorsSimple(numCopyFirst, copyDestHandle, copySrcHandle, heapType);

				copyDescriptorsSimple(numCopySecond, copyDestHandle, copySrcHandle, heapType);
			}
		}
		//拷贝目标有环绕但是拷贝源无环绕
		else
		{
			//分两个部分
			const uint32_t numCopyFirst = copyDestDistToEnd;

			const uint32_t numCopySecond = numCopy - numCopyFirst;

			copyDescriptorsSimple(numCopyFirst, copyDestHandle, copySrcHandle, heapType);

			copyDescriptorsSimple(numCopySecond, copyDestHandle, copySrcHandle, heapType);
		}
	}
	//拷贝目标无环绕
	else
	{
		//拷贝目标无环绕但是拷贝源有环绕
		if (copySrcDistToEnd < numCopy)
		{
			//分两个部分
			const uint32_t numCopyFirst = copySrcDistToEnd;

			const uint32_t numCopySecond = numCopy - numCopyFirst;

			copyDescriptorsSimple(numCopyFirst, copyDestHandle, copySrcHandle, heapType);

			copyDescriptorsSimple(numCopySecond, copyDestHandle, copySrcHandle, heapType);
		}
		//两个都没环绕，绝大多数情况
		else
		{
			copyDescriptorsSimple(numCopy, copyDestHandle, copySrcHandle, heapType);
		}
	}
}

void Gear::Core::D3D12Core::DescriptorHandle::copyDescriptorsSimple(const uint32_t numCopy, DescriptorHandle& copyDestHandle, DescriptorHandle& copySrcHandle, const D3D12_DESCRIPTOR_HEAP_TYPE heapType)
{
	GraphicsDevice::get()->CopyDescriptorsSimple(numCopy, copyDestHandle.getCurrentCPUHandle(), copySrcHandle.getCurrentCPUHandle(), heapType);

	copyDestHandle.move(numCopy);

	copySrcHandle.move(numCopy);
}
