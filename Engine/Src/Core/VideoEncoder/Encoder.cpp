#include<Gear/Core/VideoEncoder/Encoder.h>

namespace Gear::Core::VideoEncoder
{
	Encoder::Encoder(const uint32_t frameToEncode, const OutputVideoFormat format) :
		frameToEncode(frameToEncode), encodeTime(0.f), streamIndex(0), sampleDuration(10000000u / frameRate), sampleTime(0)
	{
		CHECKERROR(MFStartup(MF_VERSION));

		CHECKERROR(MFCreateSinkWriterFromURL(L"output.mp4", nullptr, nullptr, &sinkWriter));

		ComPtr<IMFMediaType> mediaType;

		CHECKERROR(MFCreateMediaType(&mediaType));

		mediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);

		switch (format)
		{
		case OutputVideoFormat::H264:
			LOGENGINE(L"输出视频格式", LogColor::brightMagenta, L"H264");
			mediaType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_H264);
			break;
		case OutputVideoFormat::HEVC:
			LOGENGINE(L"输出视频格式", LogColor::brightMagenta, L"HEVC");
			mediaType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_HEVC);
			break;
		case OutputVideoFormat::AV1:
			LOGENGINE(L"输出视频格式", LogColor::brightMagenta, L"AV1");
			mediaType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_AV1);
			break;
		default:
			LOGERROR(L"不被支持的输出视频格式！");
			break;
		}

		LOGENGINE(L"视频帧率", frameRate);

		LOGENGINE(L"待编码帧数", frameToEncode);

		MFSetAttributeSize(mediaType.Get(), MF_MT_FRAME_SIZE, Graphics::getWidth(), Graphics::getHeight());

		MFSetAttributeRatio(mediaType.Get(), MF_MT_FRAME_RATE, frameRate, 1);

		sinkWriter->AddStream(mediaType.Get(), &streamIndex);

		sinkWriter->BeginWriting();
	}

	Encoder::~Encoder()
	{
		sinkWriter->Finalize();

		sinkWriter = nullptr;

		MFShutdown();
	}

	bool Encoder::writeFrame(const void* const bitstreamPtr, const uint32_t bitstreamSize, const bool cleanPoint)
	{
		sampleTime += sampleDuration;

		ComPtr<IMFMediaBuffer> buffer;

		MFCreateMemoryBuffer(bitstreamSize, &buffer);

		BYTE* data = nullptr;

		buffer->Lock(&data, nullptr, nullptr);

		memcpy(data, bitstreamPtr, bitstreamSize);

		buffer->Unlock();

		buffer->SetCurrentLength(bitstreamSize);

		ComPtr<IMFSample> sample;

		MFCreateSample(&sample);

		sample->AddBuffer(buffer.Get());

		sample->SetSampleTime(sampleTime);

		sample->SetSampleDuration(sampleDuration);

		if (cleanPoint)
		{
			sample->SetUINT32(MFSampleExtension_CleanPoint, TRUE);
		}

		sinkWriter->WriteSample(streamIndex, sample.Get());

		displayProgress();

		return !(Graphics::getRenderedFrameCount() == frameToEncode);
	}

	void Encoder::displayProgress() const
	{
		if ((Graphics::getRenderedFrameCount() % (frameRate / 4)) == 0)
		{
			const uint32_t num = progressBarWidth * static_cast<uint32_t>(Graphics::getRenderedFrameCount()) / frameToEncode;

			const uint32_t buffLength = 12 + 2 + progressBarWidth + 1 + 6 + 1 + 1;

			wchar_t str[buffLength] = {};

			swprintf_s(str, buffLength, L"Encoding... [%.*s%.*s] %.2f%%",
				num, L"********************************",
				progressBarWidth - num, L"////////////////////////////////",
				100.f * static_cast<float>(Graphics::getRenderedFrameCount()) / static_cast<float>(frameToEncode));

			LOGENGINE(str);
		}
	}
}
