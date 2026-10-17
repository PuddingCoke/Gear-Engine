#pragma once

#ifndef _GEAR_CORE_VIDEOENCODER_ENCODER_H_
#define _GEAR_CORE_VIDEOENCODER_ENCODER_H_

#include<Gear/Core/Graphics.h>

#include<Gear/Core/VideoDevice.h>

#include<Gear/Core/D3D12Core/GraphicsCommandList.h>

#include<Gear/Core/D3D12Core/VideoProcessCommandList.h>

#include<Gear/Core/D3D12Core/CommandQueue.h>

#include<Gear/Core/D3D12Resource/ReadbackHeap.h>

extern "C"
{
#include<libavutil/avutil.h>
#include<libavformat/avformat.h>
#include<libavcodec/avcodec.h>
#include<libswscale/swscale.h>
#include<libavutil/log.h>
}

namespace Gear::Core::VideoEncoder
{
	class Encoder
	{
	public:

		enum class VideoFormat
		{
			H264, HEVC, AV1
		};

		Encoder() = delete;

		Encoder(const Encoder&) = delete;

		void operator=(const Encoder&) = delete;

		Encoder(const uint32_t frameToEncode, const VideoFormat videoFormat);

		virtual ~Encoder();

		virtual bool encode(D3D12Resource::Texture* const inputTexture) = 0;

		static constexpr uint32_t frameRate = 60;

		static constexpr uint32_t gopSeconds = 10;

		static constexpr uint32_t gopLength = frameRate * gopSeconds;

		D3D12Core::CommandQueue* getVPCommandQueue() const;

	protected:

		void writeHeader() const;

		/// <summary>
		/// 封装裸的比特流
		/// </summary>
		/// <param name="bitstreamPtr">比特流指针</param>
		/// <param name="bitstreamSize">比特流字节大小</param>
		/// <param name="syncPoint">同步点 pictureType == IDR 用于视频的正确跳转</param>
		/// <param name="decodeFrameIndex">解压帧索引必须小于等于presentFrameIndex</param>
		/// <param name="presentFrameIndex">呈现帧索引 0、1、2、3、4</param>
		/// <returns>是否继续编码</returns>
		bool writeFrame(void* const bitstreamPtr, const uint32_t bitstreamSize, const bool syncPoint,
			const int64_t decodeFrameIndex, const int64_t presentFrameIndex);

		/// <summary>
		/// 封装Packet，必须设置好packet的duration和stream_index
		/// </summary>
		/// <returns>是否继续编码</returns>
		bool writeFrame(AVPacket* const packet);

		//返回等待值
		uint64_t bgraToNV12(D3D12Resource::Texture* const inputBGRATexture, D3D12Resource::Texture* const outputNV12Texture, D3D12Core::Fence* const fence);

		uint32_t getFrameEncoded() const;

		AVStream* getOutStream() const;

		const uint32_t frameToEncode;

	private:

		void displayProgress() const;

		//不要修改这个值
		static constexpr uint32_t progressBarWidth = 32;

		uint32_t frameEncoded;

		AVFormatContext* outContext;

		AVStream* outStream;

		AVPacket* rawStreamPacket;

		ComPtr<ID3D12VideoProcessor> videoProcessor;

		D3D12Core::VideoProcessCommandListPtr vpCommandList;

		D3D12Core::CommandQueuePtr vpCommandQueue;

	};
}

#endif // !_GEAR_CORE_VIDEOENCODER_ENCODER_H_
