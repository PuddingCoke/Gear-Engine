#include<Gear/Core/Graphics.h>

#include<Gear/Core/Internal/GraphicsInternal.h>

#include<ImGUI/imgui.h>

#include<ImGUI/imgui_impl_win32.h>

#include<ImGUI/imgui_impl_dx12.h>

namespace
{
	struct GraphicsPrivate
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

		Gear::Core::Resource::ImmutableCBuffer* reservedGlobalCBuffer;

	}pvt;
}

float Gear::Core::Graphics::getExposure()
{
	return pvt.exposure;
}

void Gear::Core::Graphics::setExposure(const float exposure)
{
	pvt.exposure = exposure;
}

float Gear::Core::Graphics::getGamma()
{
	return pvt.gamma;
}

void Gear::Core::Graphics::setGamma(const float gamma)
{
	pvt.gamma = gamma;
}

uint32_t Gear::Core::Graphics::getFrameBufferCount()
{
	return pvt.frameBufferCount;
}

uint32_t Gear::Core::Graphics::getFrameIndex()
{
	return pvt.frameIndex;
}

float Gear::Core::Graphics::getDeltaTime()
{
	return pvt.deltaTime;
}

float Gear::Core::Graphics::getTimeElapsed()
{
	return pvt.timeElapsed;
}

uint32_t Gear::Core::Graphics::getWidth()
{
	return pvt.width;
}

uint32_t Gear::Core::Graphics::getHeight()
{
	return pvt.height;
}

float Gear::Core::Graphics::getAspectRatio()
{
	return pvt.aspectRatio;
}

uint64_t Gear::Core::Graphics::getRenderedFrameCount()
{
	return pvt.renderedFrameCount;
}

D3D12_CPU_DESCRIPTOR_HANDLE Gear::Core::Graphics::getBackBufferHandle()
{
	return pvt.backBufferHandle;
}

Gear::Core::Resource::ImmutableCBuffer* Gear::Core::Graphics::getReservedGlobalCBuffer()
{
	return pvt.reservedGlobalCBuffer;
}

void Gear::Core::Graphics::Internal::initialize(const uint32_t frameBufferCount, const uint32_t width, const uint32_t height)
{
	pvt.frameBufferCount = frameBufferCount;

	pvt.width = width;

	pvt.height = height;

	pvt.aspectRatio = static_cast<float>(width) / static_cast<float>(height);
}

void Gear::Core::Graphics::Internal::renderedFrameCountInc()
{
	pvt.renderedFrameCount++;
}

void Gear::Core::Graphics::Internal::setFrameIndex(const uint32_t frameIndex)
{
	pvt.frameIndex = frameIndex;
}

void Gear::Core::Graphics::Internal::setDeltaTime(const float deltaTime)
{
	pvt.deltaTime = deltaTime;
}

void Gear::Core::Graphics::Internal::updateTimeElapsed()
{
	pvt.timeElapsed += pvt.deltaTime;
}

void Gear::Core::Graphics::Internal::setBackBufferHandle(const D3D12_CPU_DESCRIPTOR_HANDLE backBufferHandle)
{
	pvt.backBufferHandle = backBufferHandle;
}

void Gear::Core::Graphics::Internal::setReservedGlobalCBuffer(Resource::ImmutableCBuffer* const reservedGlobalCBuffer)
{
	pvt.reservedGlobalCBuffer = reservedGlobalCBuffer;
}

void Gear::Core::Graphics::Internal::imGUICall()
{
	ImGui::Begin("Graphcis Settings");
	ImGui::SliderFloat("Exposure", &pvt.exposure, 0.f, 10.f);
	ImGui::SliderFloat("Gamma", &pvt.gamma, 0.f, 10.f);
	ImGui::End();
}
