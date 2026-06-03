#include<Gear/Core/Graphics.h>

#include<Gear/Core/Internal/GraphicsInternal.h>

#include<ImGUI/imgui.h>

namespace Gear::Core::Graphics
{
	namespace Internal
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

			D3D12_CPU_DESCRIPTOR_HANDLE backBufferHandle = {};

			Gear::Core::Resource::ImmutableCBuffer* engineDefinedGlobalCBuffer = nullptr;

		}impl;

		void initialize(const uint32_t frameBufferCount, const uint32_t width, const uint32_t height)
		{
			impl.frameBufferCount = frameBufferCount;

			impl.width = width;

			impl.height = height;

			impl.aspectRatio = static_cast<float>(width) / static_cast<float>(height);
		}

		void renderedFrameCountInc()
		{
			impl.renderedFrameCount++;
		}

		void setFrameIndex(const uint32_t frameIndex)
		{
			impl.frameIndex = frameIndex;
		}

		void setDeltaTime(const float deltaTime)
		{
			impl.deltaTime = deltaTime;
		}

		void updateTimeElapsed()
		{
			impl.timeElapsed += impl.deltaTime;
		}

		void setBackBufferHandle(const D3D12_CPU_DESCRIPTOR_HANDLE backBufferHandle)
		{
			impl.backBufferHandle = backBufferHandle;
		}

		void setEngineDefinedGlobalCBuffer(Resource::ImmutableCBuffer* const engineDefinedGlobalCBuffer)
		{
			impl.engineDefinedGlobalCBuffer = engineDefinedGlobalCBuffer;
		}

		void imGUICall()
		{
			ImGui::Begin("Graphcis Settings");
			ImGui::SliderFloat("Exposure", &impl.exposure, 0.f, 10.f);
			ImGui::SliderFloat("Gamma", &impl.gamma, 0.f, 10.f);
			ImGui::End();
		}
	}

	float getExposure()
	{
		return Internal::impl.exposure;
	}

	void setExposure(const float exposure)
	{
		Internal::impl.exposure = exposure;
	}

	float getGamma()
	{
		return Internal::impl.gamma;
	}

	void setGamma(const float gamma)
	{
		Internal::impl.gamma = gamma;
	}

	uint32_t getFrameBufferCount()
	{
		return Internal::impl.frameBufferCount;
	}

	uint32_t getFrameIndex()
	{
		return Internal::impl.frameIndex;
	}

	float getDeltaTime()
	{
		return Internal::impl.deltaTime;
	}

	float getTimeElapsed()
	{
		return Internal::impl.timeElapsed;
	}

	uint32_t getWidth()
	{
		return Internal::impl.width;
	}

	uint32_t getHeight()
	{
		return Internal::impl.height;
	}

	float getAspectRatio()
	{
		return Internal::impl.aspectRatio;
	}

	uint64_t getRenderedFrameCount()
	{
		return Internal::impl.renderedFrameCount;
	}

	D3D12_CPU_DESCRIPTOR_HANDLE getBackBufferHandle()
	{
		return Internal::impl.backBufferHandle;
	}

	Resource::ImmutableCBuffer* getEngineDefinedGlobalCBuffer()
	{
		return Internal::impl.engineDefinedGlobalCBuffer;
	}
}
