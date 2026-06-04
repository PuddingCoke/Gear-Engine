#pragma once

#ifndef _GEAR_CORE_D3D12CORE_ROOTSIGNATURE_H_
#define _GEAR_CORE_D3D12CORE_ROOTSIGNATURE_H_

#include<Gear/Core/GraphicsDevice.h>

#include"CommonShaderLayout.h"

namespace Gear::Core::D3D12Core
{
	class RootSignature
	{
	public:

		RootSignature() = delete;

		RootSignature(const RootSignature&) = delete;

		void operator=(const RootSignature&) = delete;

		RootSignature(
			const uint32_t numVSConstants,
			const uint32_t numHSConstants,
			const uint32_t numDSConstants,
			const uint32_t numGSConstants,
			const uint32_t numPSConstants,
			const uint32_t numCSConstants,
			CD3DX12_STATIC_SAMPLER_DESC* const samplerDescs, const uint32_t samplerCount,
			const D3D12_ROOT_SIGNATURE_FLAGS signatureFlags);

		virtual ~RootSignature();

		ID3D12RootSignature* get() const;

		static uint32_t getEngineDefinedGlobalConstantBufferParameterIndex();

		static uint32_t getUserDefinedGlobalConstantBufferParameterIndex();

		uint32_t getVSConstantsParameterIndex() const;

		uint32_t getVSConstantBufferParameterIndex() const;

		uint32_t getHSConstantsParameterIndex() const;

		uint32_t getHSConstantBufferParameterIndex() const;

		uint32_t getDSConstantsParameterIndex() const;

		uint32_t getDSConstantBufferParameterIndex() const;

		uint32_t getGSConstantsParameterIndex() const;

		uint32_t getGSConstantBufferParameterIndex() const;

		uint32_t getPSConstantsParameterIndex() const;

		uint32_t getPSConstantBufferParameterIndex() const;

		uint32_t getCSConstantsParameterIndex() const;

		uint32_t getCSConstantBufferParameterIndex() const;

	private:

		enum class ShaderType
		{
			VERTEX,
			HULL,
			DOMAIN,
			GEOMETRY,
			PIXEL,
			COMPUTE,
			TYPECOUNT
		};

		//V H G D P C，与ShaderType中枚举表示的整数值一一对应
		CommonShaderLayout::ShaderLocalParameterIndices localParameterIndices[static_cast<uint32_t>(ShaderType::TYPECOUNT)];

		CommonShaderLayout::ShaderLocalParameterIndices getLocalParameterIndices(const ShaderType shaderType) const;

		uint32_t getLocalConstantsParameterIndex(const ShaderType shaderType) const;

		uint32_t getLocalConstantBufferParameterIndex(const ShaderType shaderType) const;

		ComPtr<ID3D12RootSignature> rootSignature;

	};
}

#endif // !_GEAR_CORE_D3D12CORE_ROOTSIGNATURE_H_