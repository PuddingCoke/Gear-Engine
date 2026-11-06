#include <Gear/Core/D3D12Core/Fence.h>

namespace Gear::Core::D3D12Core
{
	Fence::Fence(const D3D12_FENCE_FLAGS flags) :
		fenceEvent(CreateEvent(nullptr, FALSE, FALSE, nullptr)), currentFenceValue(0ull)
	{
		GraphicsDevice::get()->CreateFence(currentFenceValue, flags, IID_PPV_ARGS(&fence));
	}

	Fence::~Fence()
	{
		if (fenceEvent)
		{
			CloseHandle(fenceEvent);
		}
	}

	ID3D12Fence* Fence::get() const
	{
		return fence.Get();
	}

	uint64_t Fence::increment()
	{
		return ++currentFenceValue;
	}

	void Fence::waitValue(const uint64_t value)
	{
		fence->SetEventOnCompletion(value, fenceEvent);

		WaitForSingleObjectEx(fenceEvent, INFINITE, FALSE);
	}

	uint64_t Fence::signal(ID3D12CommandQueue* const commandQueue)
	{
		const uint64_t signalValue = increment();

		commandQueue->Signal(fence.Get(), signalValue);

		return signalValue;
	}

	void Fence::wait(ID3D12CommandQueue* const commandQueue, const uint64_t waitValue)
	{
		commandQueue->Wait(fence.Get(), waitValue);
	}

	uint64_t Fence::getCompletedValue() const
	{
		return fence->GetCompletedValue();
	}
}
