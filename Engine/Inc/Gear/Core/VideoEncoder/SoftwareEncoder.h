#pragma once

#ifndef _GEAR_CORE_VIDEOENCODER_SOFTWAREENCODER_H_
#define _GEAR_CORE_VIDEOENCODER_SOFTWAREENCODER_H_

#include "Encoder.h"

namespace Gear::Core::VideoEncoder
{
	class SoftwareEncoder : public Encoder
	{
	public:

		SoftwareEncoder(const uint32_t frameToEncode);

		~SoftwareEncoder();

		bool encode(D3D12Resource::Texture* const inputTexture) override;

	private:

		static constexpr VideoFormat videoFormat = VideoFormat::H264;

		static constexpr AVPixelFormat sourceFormat = AV_PIX_FMT_NV12;

		static constexpr AVPixelFormat destFormat = AV_PIX_FMT_YUV420P;

		AVCodecContext* codecContext = nullptr;

		AVFrame* yuvFrame = nullptr;

		SwsContext* swsContext = nullptr;

		AVPacket* packet = nullptr;

		D3D12Core::GraphicsCommandListPtr cpCommandList;

		D3D12Core::CommandQueuePtr cpCommandQueue;

		D3D12Core::FencePtr cpSyncFence;

		D3D12Resource::TexturePtr nv12Texture;

		D3D12Resource::ReadbackHeapPtr nv12ReadbackHeap;

		D3D12_PLACED_SUBRESOURCE_FOOTPRINT cpFootPrints[2];
	};
}

#endif // !_GEAR_CORE_VIDEOENCODER_SOFTWAREENCODER_H_
