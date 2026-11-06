#include <Gear/Core/Graphics.h>

#include <Gear/Core/Internal/GraphicsInternal.h>

#include <ImGUI/imgui.h>

namespace Gear::Core::Graphics
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

		float frameRate = 0.f;

	} impl;

	namespace Internal
	{
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

		void setFrameRate(const float frameRate)
		{
			impl.frameRate = frameRate;
		}

		void imGuiCall()
		{
			ImGui::Begin("Graphcis Settings");
			ImGui::SliderFloat("Exposure", &impl.exposure, 0.f, 10.f);
			ImGui::SliderFloat("Gamma", &impl.gamma, 0.f, 10.f);
			ImGui::End();
		}
	}

	float getExposure()
	{
		return impl.exposure;
	}

	void setExposure(const float exposure)
	{
		impl.exposure = exposure;
	}

	float getGamma()
	{
		return impl.gamma;
	}

	void setGamma(const float gamma)
	{
		impl.gamma = gamma;
	}

	uint32_t getFrameBufferCount()
	{
		return impl.frameBufferCount;
	}

	uint32_t getFrameIndex()
	{
		return impl.frameIndex;
	}

	float getDeltaTime()
	{
		return impl.deltaTime;
	}

	float getTimeElapsed()
	{
		return impl.timeElapsed;
	}

	float getFrameRate()
	{
		return impl.frameRate;
	}

	uint32_t getWidth()
	{
		return impl.width;
	}

	uint32_t getHeight()
	{
		return impl.height;
	}

	float getAspectRatio()
	{
		return impl.aspectRatio;
	}

	uint64_t getRenderedFrameCount()
	{
		return impl.renderedFrameCount;
	}
}
