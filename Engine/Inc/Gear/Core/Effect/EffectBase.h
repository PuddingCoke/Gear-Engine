#pragma once

#ifndef _GEAR_CORE_EFFECT_EFFECTBASE_H_
#define _GEAR_CORE_EFFECT_EFFECTBASE_H_

#include<Gear/Core/ResourceManager.h>

#include<Gear/Core/D3D12Core/Shader.h>

#include<Gear/Core/GlobalShader.h>

#include<Gear/Core/PipelineStateBuilder.h>

#include<Gear/Core/PipelineStateHelper.h>

#include<ImGUI/imgui.h>

namespace Gear::Core::Effect
{
	class EffectBase
	{
	public:

		EffectBase() = delete;

		EffectBase(const EffectBase&) = delete;

		void operator=(const EffectBase&) = delete;

		EffectBase(GraphicsContext* const context, const uint32_t width, const uint32_t height, const DXGI_FORMAT format);

		EffectBase(GraphicsContext* const context, const uint32_t width, const uint32_t height, const DXGI_FORMAT resFormat, const uint32_t arraySize, const uint32_t mipLevels, const bool isTextureCube,
			const DXGI_FORMAT srvFormat, const DXGI_FORMAT uavFormat, const DXGI_FORMAT rtvFormat);

		virtual ~EffectBase();

		virtual void imGUICall() = 0;

	protected:

		UniquePtr<Resource::TextureRenderView> outputTexture;

		//引用
		GraphicsContext* const context;

		const uint32_t width;

		const uint32_t height;

	};
}

#endif // !_GEAR_CORE_EFFECT_EFFECTBASE_H_