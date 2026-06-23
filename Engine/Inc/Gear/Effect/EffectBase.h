#pragma once

#ifndef _GEAR_EFFECT_EFFECTBASE_H_
#define _GEAR_EFFECT_EFFECTBASE_H_

#include<Gear/Core/ResourceManager.h>

#include<Gear/Core/D3D12Core/Shader.h>

#include<Gear/Core/GlobalShader.h>

#include<Gear/Core/PipelineStateBuilder.h>

#include<Gear/Core/PipelineStateHelper.h>

#include<ImGUI/imgui.h>

namespace Gear::Effect
{
	using namespace Gear::Core;

	using namespace Gear::Resource;

	using namespace Core::D3D12Core;

	class EffectBase
	{
	public:

		EffectBase() = delete;

		EffectBase(const EffectBase&) = delete;

		void operator=(const EffectBase&) = delete;

		EffectBase(GraphicsContext& contextRef, const uint32_t width, const uint32_t height, const DXGI_FORMAT format);

		EffectBase(GraphicsContext& contextRef, const uint32_t width, const uint32_t height, const DXGI_FORMAT resFormat, const uint32_t arraySize, const uint32_t mipLevels, const bool isTextureCube,
			const DXGI_FORMAT srvFormat, const DXGI_FORMAT uavFormat, const DXGI_FORMAT rtvFormat);

		virtual ~EffectBase();

		virtual void imGuiCall() = 0;

	protected:

		RenderTextureViewPtr outputTexture;

		//引用
		GraphicsContext* const context;

		const uint32_t width;

		const uint32_t height;

	};
}

#endif // !_GEAR_EFFECT_EFFECTBASE_H_