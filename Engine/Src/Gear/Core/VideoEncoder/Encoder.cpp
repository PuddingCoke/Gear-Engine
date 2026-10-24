#include<Gear/Core/VideoEncoder/Encoder.h>

#include<Gear/Utils/Math.h>

namespace Gear::Core::VideoEncoder
{
	void logCallback(void* avcl, int level, const char* fmt, va_list vl)
	{
		if (level > av_log_get_level())
		{
			return;
		}

		char buff[4096] = {};

		int printPrefix = 1;

		av_log_format_line(avcl, level, fmt, vl, buff, sizeof(buff), &printPrefix);

		const size_t len = strlen(buff);

		if (len && buff[len - 1] == '\n')
		{
			buff[len - 1] = '\0';
		}

		LOGENGINE() << buff;
	}

	Encoder::Encoder(const uint32_t frameToEncode, const VideoFormat videoFormat) :
		frameEncoded(0), frameToEncode(frameToEncode)
	{
		av_log_set_callback(logCallback);

		avformat_network_init();

		const Utils::File::UserDataPath outputPath = L"output.mp4";

		const std::string multibytePath = outputPath.multibyteStr();

		avformat_alloc_output_context2(&outContext, nullptr, "mp4", multibytePath.c_str());

		outStream = avformat_new_stream(outContext, nullptr);

		outStream->time_base = AVRational{ 1, AV_TIME_BASE };

		outStream->id = 0;

		LOGENGINE() << "视频路径" << outputPath;

		LOGENGINE() << "视频时间" << LogFloatPrecision(1) << static_cast<float>(frameToEncode) / static_cast<float>(frameRate) << "秒";

		LOGENGINE() << "视频帧率" << frameRate;

		AVCodecParameters* const param = outStream->codecpar;

		switch (videoFormat)
		{
		case VideoFormat::H264:
			LOGENGINE() << "视频格式" << COLORIZEENUM(AV_CODEC_ID_H264);
			param->codec_id = AV_CODEC_ID_H264;
			break;
		case VideoFormat::HEVC:
			LOGENGINE() << "视频格式" << COLORIZEENUM(AV_CODEC_ID_HEVC);
			param->codec_id = AV_CODEC_ID_HEVC;
			break;
		case VideoFormat::AV1:
			LOGENGINE() << "视频格式" << COLORIZEENUM(AV_CODEC_ID_AV1);
			param->codec_id = AV_CODEC_ID_AV1;
			break;
		default:
			THROWLOG(LOGERROR() << "不被支持的视频格式！");
			break;
		}

		param->codec_type = AVMEDIA_TYPE_VIDEO;

		param->width = static_cast<int32_t>(Graphics::getWidth());

		param->height = static_cast<int32_t>(Graphics::getHeight());

		param->framerate = AVRational{ static_cast<int32_t>(frameRate),1 };

		avio_open(&outContext->pb, outContext->url, AVIO_FLAG_WRITE);

		rawStreamPacket = av_packet_alloc();

		LOGENGINE() << "待编码帧数" << frameToEncode;

		LOGENGINE() << "开始编码";

		D3D12_VIDEO_PROCESS_INPUT_STREAM_DESC inputDesc =
		{
			.Format = Graphics::backBufferFormat,
			.ColorSpace = DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709,
			.FrameRate = { frameRate, 1 },
			.SourceSizeRange = { Graphics::getWidth(), Graphics::getHeight(), Graphics::getWidth(), Graphics::getHeight() },
			.DestinationSizeRange = { Graphics::getWidth(), Graphics::getHeight(), Graphics::getWidth(), Graphics::getHeight() },
		};

		D3D12_VIDEO_PROCESS_OUTPUT_STREAM_DESC outputDesc =
		{
		.Format = DXGI_FORMAT_NV12,
		.ColorSpace = DXGI_COLOR_SPACE_YCBCR_STUDIO_G22_LEFT_P709,
		.FrameRate = { frameRate, 1 },
		};

		CHECKERROR(VideoDevice::get()->CreateVideoProcessor(0, &outputDesc, 1, &inputDesc, IID_PPV_ARGS(&videoProcessor)));

		vpCommandList = makeUnique<D3D12Core::VideoProcessCommandList>();

		vpCommandQueue = makeUnique<D3D12Core::CommandQueue>(vpCommandList.get());
	}

	Encoder::~Encoder()
	{
		if (vpCommandQueue)
		{
			vpCommandQueue->waitDestroyable();
		}

		if (rawStreamPacket)
		{
			av_packet_free(&rawStreamPacket);
		}

		av_write_trailer(outContext);

		avio_close(outContext->pb);

		avformat_free_context(outContext);

		avformat_network_deinit();
	}

	D3D12Core::CommandQueue* Encoder::getVPCommandQueue() const
	{
		return vpCommandQueue.get();
	}

	void Encoder::writeHeader() const
	{
		avformat_write_header(outContext, nullptr);
	}

	bool Encoder::writeFrame(void* const bitstreamPtr, const uint32_t bitstreamSize, const bool syncPoint,
		const int64_t decodeFrameIndex, const int64_t presentFrameIndex)
	{
		rawStreamPacket->pts = av_rescale_q(presentFrameIndex, AVRational{ 1,static_cast<int32_t>(frameRate) }, outStream->time_base);

		rawStreamPacket->dts = av_rescale_q(decodeFrameIndex, AVRational{ 1,static_cast<int32_t>(frameRate) }, outStream->time_base);

		rawStreamPacket->duration = av_rescale_q(1, AVRational{ 1,static_cast<int32_t>(frameRate) }, outStream->time_base);

		rawStreamPacket->stream_index = outStream->index;

		rawStreamPacket->data = static_cast<uint8_t*>(bitstreamPtr);

		rawStreamPacket->size = bitstreamSize;

		if (syncPoint)
		{
			rawStreamPacket->flags |= AV_PKT_FLAG_KEY;
		}

		av_interleaved_write_frame(outContext, rawStreamPacket);

		av_packet_unref(rawStreamPacket);

		frameEncoded++;

		displayProgress();

		return frameEncoded != frameToEncode;
	}

	bool Encoder::writeFrame(AVPacket* const packet)
	{
		av_interleaved_write_frame(outContext, packet);

		frameEncoded++;

		displayProgress();

		return frameEncoded != frameToEncode;
	}

	uint64_t Encoder::bgraToNV12(D3D12Resource::Texture* const inputBGRATexture, D3D12Resource::Texture* const outputNV12Texture, D3D12Core::Fence* const fence)
	{
		vpCommandQueue->begin();

		D3D12Core::VPInputArguments inputArgs = D3D12Core::VPInputArguments(inputBGRATexture);

		D3D12Core::VPOutputArguments outputArgs = D3D12Core::VPOutputArguments(outputNV12Texture);

		vpCommandList->processFrames(videoProcessor.Get(), outputArgs, { inputArgs });

		vpCommandQueue->processCommandLists();

		vpCommandQueue->waitFrameGPUComplete();

		return vpCommandQueue->signal(fence);
	}

	uint32_t Encoder::getFrameEncoded() const
	{
		return frameEncoded;
	}

	AVStream* Encoder::getOutStream() const
	{
		return outStream;
	}

	void Encoder::displayProgress() const
	{
		if ((frameEncoded % (frameRate / 4)) == 0)
		{
			const float progress = Utils::Math::saturate(static_cast<float>(frameEncoded) / static_cast<float>(frameToEncode));

			const uint32_t num = static_cast<uint32_t>(Utils::Math::clamp(static_cast<int32_t>(progressBarWidth * progress), 0, static_cast<int32_t>(progressBarWidth)));

			const uint32_t buffLength = 13 + 2 + progressBarWidth + 1 + 6 + 1 + 1 + 8;

			char str[buffLength] = {};

			sprintf_s(str, buffLength, "编码中... [%.*s%.*s] %.2f%%",
				num, "********************************",
				progressBarWidth - num, "////////////////////////////////",
				100.f * progress);

			LOGENGINE() << str;
		}
	}
}
