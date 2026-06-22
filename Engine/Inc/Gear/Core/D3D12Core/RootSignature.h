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

		static uint32_t getEngineGlobalConstantBufferParameterIndex();

		static uint32_t getUserGlobalConstantBufferParameterIndex();

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

		uint32_t getNumShaderConstants(const ShaderType shaderType) const;

		static constexpr uint32_t maxDWORD = 64u;

		static constexpr uint32_t perConstantDWORD = 1u;

		static constexpr uint32_t perDescriptorDWORD = 2u;

		//32个常量已经足够慷慨了，总计32个DWORD
		static constexpr uint32_t maxPerShaderConstants = 32u;

	private:

		std::array<uint32_t, static_cast<uint32_t>(ShaderType::TYPECOUNT)> numShaderConstants;

		//V H G D P C，与ShaderType中枚举表示的整数值一一对应
		CommonShaderLayout::ShaderLocalParameterIndices localParameterIndices[static_cast<uint32_t>(ShaderType::TYPECOUNT)];

		CommonShaderLayout::ShaderLocalParameterIndices getLocalParameterIndices(const ShaderType shaderType) const;

		uint32_t getLocalConstantsParameterIndex(const ShaderType shaderType) const;

		uint32_t getLocalConstantBufferParameterIndex(const ShaderType shaderType) const;

		ComPtr<ID3D12RootSignature> rootSignature;

	};
}

#endif // !_GEAR_CORE_D3D12CORE_ROOTSIGNATURE_H_