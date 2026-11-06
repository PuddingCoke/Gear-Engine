#include <Gear/Core/VideoEncoder/SoftwareEncoder.h>

namespace Gear::Core::VideoEncoder
{
	SoftwareEncoder::SoftwareEncoder(const uint32_t frameToEncode) : Encoder(frameToEncode, videoFormat)
	{
		const AVCodec* codec = avcodec_find_encoder_by_name("libx264");

		codecContext = avcodec_alloc_context3(codec);

		codecContext->width = Graphics::getWidth();

		codecContext->height = Graphics::getHeight();

		codecContext->time_base = AVRational{ 1, static_cast<int>(frameRate) };

		codecContext->framerate = AVRational{ static_cast<int32_t>(frameRate), 1 };

		codecContext->pix_fmt = destFormat;

		codecContext->gop_size = static_cast<int>(gopLength);

		codecContext->color_primaries = AVCOL_PRI_BT709;

		codecContext->color_trc = AVCOL_TRC_GAMMA22;

		codecContext->colorspace = AVCOL_SPC_BT709;

		codecContext->color_range = AVCOL_RANGE_MPEG;

		AVDictionary* opts = nullptr;

		av_dict_set(&opts, "preset", "veryslow", 0);

		av_dict_set(&opts, "crf", "18", 0);

		avcodec_open2(codecContext, codec, &opts);

		av_dict_free(&opts);

		avcodec_parameters_from_context(getOutStream()->codecpar, codecContext);

		swsContext = sws_getContext(codecContext->width,
		                            codecContext->height,
		                            sourceFormat,
		                            codecContext->width,
		                            codecContext->height,
		                            destFormat,
		                            SWS_POINT,
		                            nullptr,
		                            nullptr,
		                            nullptr);

		yuvFrame = av_frame_alloc();

		yuvFrame->format = codecContext->pix_fmt;

		yuvFrame->width = codecContext->width;

		yuvFrame->height = codecContext->height;

		av_frame_get_buffer(yuvFrame, 0);

		packet = av_packet_alloc();

		writeHeader();

		cpCommandList = makeUnique<D3D12Core::GraphicsCommandList>(D3D12_COMMAND_LIST_TYPE_COPY);

		cpCommandQueue = makeUnique<D3D12Core::CommandQueue>(cpCommandList.get());

		cpSyncFence = makeUnique<D3D12Core::Fence>();

		nv12Texture = makeUnique<D3D12Resource::Texture>(Graphics::getWidth(),
		                                                 Graphics::getHeight(),
		                                                 FMT::NV12,
		                                                 1,
		                                                 1,
		                                                 true,
		                                                 D3D12_RESOURCE_FLAG_NONE,
		                                                 nullptr,
		                                                 D3D12_RESOURCE_STATE_COMMON);

		{
			uint64_t totalBytes;

			const D3D12_RESOURCE_DESC resourceDesc = nv12Texture->getResource()->GetDesc();

			GraphicsDevice::get()->GetCopyableFootprints(
				&resourceDesc, 0, 2, 0, cpFootPrints, nullptr, nullptr, &totalBytes);

			nv12ReadbackHeap = makeUnique<D3D12Resource::ReadbackHeap>(totalBytes);
		}
	}

	SoftwareEncoder::~SoftwareEncoder()
	{
		avcodec_send_frame(codecContext, nullptr);

		while (avcodec_receive_packet(codecContext, packet) == 0)
		{
			av_packet_unref(packet);
		}

		av_packet_free(&packet);

		av_frame_free(&yuvFrame);

		sws_freeContext(swsContext);

		avcodec_free_context(&codecContext);
	}

	bool SoftwareEncoder::encode(D3D12Resource::Texture* const inputTexture)
	{
		const uint64_t waitValue = bgraToNV12(inputTexture, nv12Texture.get(), cpSyncFence.get());

		cpCommandQueue->wait(cpSyncFence.get(), waitValue);

		cpCommandQueue->begin();

		cpCommandList->trackAndSetResourceState(nv12Texture.get(),
		                                        D3D12Resource::D3D12_TRANSITION_ALL_MIPLEVELS,
		                                        D3D12_RESOURCE_STATE_COPY_SOURCE);

		cpCommandList->flushResourceBarriers();

		for (uint32_t i = 0; i < 2; i++)
		{
			const CD3DX12_TEXTURE_COPY_LOCATION copyDest(nv12ReadbackHeap->getResource(), cpFootPrints[i]);

			const CD3DX12_TEXTURE_COPY_LOCATION copySrc(nv12Texture->getResource(),
			                                            D3D12CalcSubresource(0, 0, i, 1, 1));

			cpCommandList->get()->CopyTextureRegion(&copyDest, 0, 0, 0, &copySrc, nullptr);
		}

		cpCommandList->trackAndSetResourceState(
			nv12Texture.get(), D3D12Resource::D3D12_TRANSITION_ALL_MIPLEVELS, D3D12_RESOURCE_STATE_COMMON);

		cpCommandList->flushResourceBarriers();

		cpCommandQueue->processCommandLists();

		cpCommandQueue->waitFrameGPUComplete();

		const uint8_t* const data = reinterpret_cast<const uint8_t*>(nv12ReadbackHeap->map());

		const uint8_t* sourceData[2] = { data + cpFootPrints[0].Offset, data + cpFootPrints[1].Offset };

		const int32_t sourceStride[2] = { static_cast<int32_t>(cpFootPrints[0].Footprint.RowPitch),
			                              static_cast<int32_t>(cpFootPrints[1].Footprint.RowPitch) };

		sws_scale(swsContext,
		          sourceData,
		          sourceStride,
		          0,
		          codecContext->height,
		          yuvFrame->data,
		          yuvFrame->linesize);

		//强制尾帧为P帧（编码器有可能覆盖为I帧，但不可能为B帧）
		//因为libx264可能不会以编码顺序输出编码后的比特流
		//另外，这也可以支持循环动画
		yuvFrame->pict_type =
			((Graphics::getRenderedFrameCount() == frameToEncode - 1u) ? AV_PICTURE_TYPE_P :
		                                                                 AV_PICTURE_TYPE_NONE);

		//YUV帧的呈现时间戳应该等于已渲染帧数
		yuvFrame->pts = static_cast<int64_t>(Graphics::getRenderedFrameCount());

		avcodec_send_frame(codecContext, yuvFrame);

		while (avcodec_receive_packet(codecContext, packet) == 0)
		{
			//抛弃在视频时间轴之外的packet
			//这么做需要强制末尾几帧为P或I帧
			if (packet->pts < static_cast<int64_t>(frameToEncode))
			{
				av_packet_rescale_ts(packet, codecContext->time_base, getOutStream()->time_base);

				packet->duration = av_rescale_q(
					1, AVRational{ 1, static_cast<int32_t>(frameRate) }, getOutStream()->time_base);

				packet->stream_index = getOutStream()->index;

				if (!writeFrame(packet))
				{
					av_packet_unref(packet);

					nv12ReadbackHeap->unmap();

					return false;
				}
			}

			av_packet_unref(packet);
		}

		nv12ReadbackHeap->unmap();

		return true;
	}
}
