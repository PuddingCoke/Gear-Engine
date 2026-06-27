#pragma once

#ifndef _GEAR_CORE_VIDEOENCODER_NVIDIAENCODER_H_
#define _GEAR_CORE_VIDEOENCODER_NVIDIAENCODER_H_

#include<Gear/Core/D3D12Resource/ReadbackHeap.h>

#include"Encoder.h"

#include<NvEnc/nvEncodeAPI.h>

#include<queue>

//基本的工作流程

//准备
//1.打开编码会议
//2.初始化编码器

//逐帧编码的流程
//1.注册输入资源
//2.映射输入资源
//3.注册输出资源
//4.映射输出资源
//5.编码图像
//6.锁定比特流
//7.获得比特流指针
//8.解锁比特流
//9.解除输入资源映射
//10.解除输入资源注册
//11.解除输出资源映射
//12.解除输出资源注册

namespace Gear::Core::VideoEncoder
{
	class NVIDIAEncoder :public Encoder
	{
	public:

		NVIDIAEncoder() = delete;

		NVIDIAEncoder(const NVIDIAEncoder&) = delete;

		NVIDIAEncoder(const uint32_t frameToEncode);

		~NVIDIAEncoder();

		bool encode(D3D12Resource::Texture* const inputTexture) override;

		static constexpr uint32_t lookaheadDepth = 31;

	private:

		static constexpr NV_ENC_BUFFER_FORMAT bufferFormat = NV_ENC_BUFFER_FORMAT_ARGB;

		static constexpr NV_ENC_TUNING_INFO tuningInfo = NV_ENC_TUNING_INFO_HIGH_QUALITY;

		static constexpr OutputVideoFormat outputVideoFormat = OutputVideoFormat::H264;

		const GUID codec = NV_ENC_CODEC_H264_GUID;

		const GUID preset = NV_ENC_PRESET_P7_GUID;

		const GUID profile = NV_ENC_H264_PROFILE_HIGH_GUID;

		HMODULE moduleNvEncAPI;

		NV_ENCODE_API_FUNCTION_LIST nvencAPI;

		void* encoder;

		UniquePtr<D3D12Resource::ReadbackHeap> readbackHeap;

		ComPtr<ID3D12Fence> outputFence;

		uint32_t outputFenceValue;

		std::queue<NV_ENC_REGISTERED_PTR> registeredInputResourcePtrs;

		std::queue<NV_ENC_INPUT_PTR> mappedInputResourcePtrs;

		std::queue<NV_ENC_OUTPUT_RESOURCE_D3D12> outputResources;

		NV_ENC_REGISTERED_PTR registeredOutputResourcePtr;

		NV_ENC_INPUT_PTR mappedOutputResourcePtr;

	};
}

#endif // !_GEAR_CORE_VIDEOENCODER_NVIDIAENCODER_H_