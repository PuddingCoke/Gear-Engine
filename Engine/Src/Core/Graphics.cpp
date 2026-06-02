#include<Gear/Core/Graphics.h>

#include<Gear/Core/Internal/GraphicsInternal.h>

#include<ImGUI/imgui.h>

#include<ImGUI/imgui_impl_win32.h>

#include<ImGUI/imgui_impl_dx12.h>

namespace
{
	struct GraphicsImpl
	{

		float exposure = 1.f;

		float gamma = 2.2f;

		uint32_t frameBufferCount = 0;

		uint32_t frameIndex = 0;

		uint32_t width = 0;

		uint32_t height = 0;

		uint64_t renderedFrameCount = 0;

		float aspectRatio = 0.f;

		float deltaTime = 0.f;

		float timeElapsed = 0.f;

		D3D12_CPU_DESCRIPTOR_HANDLE backBufferHandle;

		Gear::Core::Resource::ImmutableCBuffer* engineDefinedGlobalCBuffer;

	}impl;
}

float Gear::Core::Graphics::getExposure()
{
	return impl.exposure;
}

void Gear::Core::Graphics::setExposure(const float exposure)
{
	impl.exposure = exposure;
}

float Gear::Core::Graphics::getGamma()
{
	return impl.gamma;
}

void Gear::Core::Graphics::setGamma(const float gamma)
{
	impl.gamma = gamma;
}

uint32_t Gear::Core::Graphics::getFrameBufferCount()
{
	return impl.frameBufferCount;
}

uint32_t Gear::Core::Graphics::getFrameIndex()
{
	return impl.frameIndex;
}

float Gear::Core::Graphics::getDeltaTime()
{
	return impl.deltaTime;
}

float Gear::Core::Graphics::getTimeElapsed()
{
	return impl.timeElapsed;
}

uint32_t Gear::Core::Graphics::getWidth()
{
	return impl.width;
}

uint32_t Gear::Core::Graphics::getHeight()
{
	return impl.height;
}

float Gear::Core::Graphics::getAspectRatio()
{
	return impl.aspectRatio;
}

uint64_t Gear::Core::Graphics::getRenderedFrameCount()
{
	return impl.renderedFrameCount;
}

D3D12_CPU_DESCRIPTOR_HANDLE Gear::Core::Graphics::getBackBufferHandle()
{
	return impl.backBufferHandle;
}

Gear::Core::Resource::ImmutableCBuffer* Gear::Core::Graphics::getEngineDefinedGlobalCBuffer()
{
	return impl.engineDefinedGlobalCBuffer;
}

void Gear::Core::Graphics::Internal::initialize(const uint32_t frameBufferCount, const uint32_t width, const uint32_t height)
{
	impl.frameBufferCount = frameBufferCount;

	impl.width = width;

	impl.height = height;

	impl.aspectRatio = static_cast<float>(width) / static_cast<float>(height);
}

void Gear::Core::Graphics::Internal::renderedFrameCountInc()
{
	impl.renderedFrameCount++;
}

void Gear::Core::Graphics::Internal::setFrameIndex(const uint32_t frameIndex)
{
	impl.frameIndex = frameIndex;
}

void Gear::Core::Graphics::Internal::setDeltaTime(const float deltaTime)
{
	impl.deltaTime = deltaTime;
}

void Gear::Core::Graphics::Internal::updateTimeElapsed()
{
	impl.timeElapsed += impl.deltaTime;
}

void Gear::Core::Graphics::Internal::setBackBufferHandle(const D3D12_CPU_DESCRIPTOR_HANDLE backBufferHandle)
{
	impl.backBufferHandle = backBufferHandle;
}

void Gear::Core::Graphics::Internal::setEngineDefinedGlobalCBuffer(Resource::ImmutableCBuffer* const engineDefinedGlobalCBuffer)
{
	impl.engineDefinedGlobalCBuffer = engineDefinedGlobalCBuffer;
}

void Gear::Core::Graphics::Internal::imGUICall()
{
	ImGui::Begin("Graphcis Settings");
	ImGui::SliderFloat("Exposure", &impl.exposure, 0.f, 10.f);
	ImGui::SliderFloat("Gamma", &impl.gamma, 0.f, 10.f);
	ImGui::End();
}
