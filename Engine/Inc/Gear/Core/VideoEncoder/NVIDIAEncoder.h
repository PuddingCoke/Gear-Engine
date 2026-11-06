#pragma once

#ifndef _GEAR_CORE_VIDEOENCODER_NVIDIAENCODER_H_
#define _GEAR_CORE_VIDEOENCODER_NVIDIAENCODER_H_

#include "Encoder.h"

#include <NvEnc/nvEncodeAPI.h>

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
	class NVIDIAEncoder : public Encoder
	{
	public:

		NVIDIAEncoder() = delete;

		NVIDIAEncoder(const NVIDIAEncoder&) = delete;

		NVIDIAEncoder(const uint32_t frameToEncode, const uint32_t maxBFrames);

		~NVIDIAEncoder();

		bool encode(D3D12Resource::Texture* const inputTexture) override;

	private:

		void workerLoop();

		static constexpr uint32_t lookaheadDepth = 31;

		static constexpr uint32_t extraOutput = 8;

		static constexpr NV_ENC_BUFFER_FORMAT bufferFormat = NV_ENC_BUFFER_FORMAT_NV12;

		static constexpr NV_ENC_TUNING_INFO tuningInfo = NV_ENC_TUNING_INFO_HIGH_QUALITY;

		static constexpr VideoFormat videoFormat = VideoFormat::H264;

		const GUID codec = NV_ENC_CODEC_H264_GUID;

		const GUID preset = NV_ENC_PRESET_P7_GUID;

		const GUID profile = NV_ENC_H264_PROFILE_HIGH_GUID;

		const uint64_t readbackHeapSize = 3 * Graphics::getWidth() * Graphics::getHeight();

		const uint32_t frameIntervalP;

		const uint64_t numNV12Textures;

		HMODULE moduleNvEncAPI;

		NV_ENCODE_API_FUNCTION_LIST nvencAPI;

		void* encoder;

		D3D12Core::FencePtr inputFence;

		D3D12Core::FencePtr outputFence;

		HANDLE eosEvent;

		UniquePtr<HANDLE[]> completionEvents;

		UniquePtr<NV_ENC_OUTPUT_RESOURCE_D3D12[]> outputResources;

		UniquePtr<uint64_t[]> decodeFrameIndices;

		UniquePtr<NV_ENC_INPUT_PTR[]> mappedInputResourcePtrs;

		UniquePtr<NV_ENC_INPUT_PTR[]> mappedOutputResourcePtrs;

		UniquePtr<NV_ENC_REGISTERED_PTR[]> registeredInputResourcePtrs;

		UniquePtr<NV_ENC_REGISTERED_PTR[]> registeredOutputResourcePtrs;

		UniquePtr<D3D12Resource::TexturePtr[]> nv12Textures;

		UniquePtr<D3D12Resource::ReadbackHeapPtr[]> readbackHeaps;

		std::mutex readIndexMutex;

		std::mutex writeIndexMutex;

		std::condition_variable readCV;

		std::condition_variable writeCV;

		uint64_t writeIndex;

		uint64_t readIndex;

		std::thread workerThread;

		std::atomic<bool> encoding;
	};
}

#endif // !_GEAR_CORE_VIDEOENCODER_NVIDIAENCODER_H_
