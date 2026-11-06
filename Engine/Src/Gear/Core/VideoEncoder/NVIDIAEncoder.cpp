#include <Gear/Core/VideoEncoder/NVIDIAEncoder.h>

#define NVENCCALL(func) \
	{ \
		const NVENCSTATUS _status_ = func; \
		if (_status_ != NV_ENC_SUCCESS && _status_ != NV_ENC_ERR_LOCK_BUSY && \
		    _status_ != NV_ENC_ERR_NEED_MORE_INPUT && _status_ != NV_ENC_ERR_NEED_MORE_OUTPUT) \
		{ \
			const std::string errorString = nvencAPI.nvEncGetLastErrorString(encoder); \
			THROWLOG(LOGERROR() << "调用" << #func << "时发生错误，错误码" \
			                    << static_cast<uint32_t>(_status_) << "错误信息" << errorString); \
		} \
	}

namespace Gear::Core::VideoEncoder
{
	NVIDIAEncoder::NVIDIAEncoder(const uint32_t frameToEncode, const uint32_t maxBFrames) :
		Encoder(frameToEncode, videoFormat),
		moduleNvEncAPI(nullptr),
		nvencAPI{ NV_ENCODE_API_FUNCTION_LIST_VER },
		encoder(nullptr),
		frameIntervalP(maxBFrames + 1u),
		numNV12Textures(lookaheadDepth + frameIntervalP + extraOutput),
		inputFence(makeUnique<D3D12Core::Fence>()),
		outputFence(makeUnique<D3D12Core::Fence>()),
		readIndex(0ull),
		writeIndex(0ull),
		encoding(true)
	{
		LOGENGINE() << "最多B帧" << maxBFrames;

		moduleNvEncAPI = LoadLibraryA("nvEncodeAPI64.dll");

		if (moduleNvEncAPI == nullptr)
		{
			THROWLOG(LOGERROR() << "无法读取nvEncodeAPI64.dll！");
		}

		NVENCSTATUS(__stdcall * NVENCAPICreateInstance)(NV_ENCODE_API_FUNCTION_LIST*) =
			(NVENCSTATUS (*)(NV_ENCODE_API_FUNCTION_LIST*))GetProcAddress(moduleNvEncAPI,
		                                                                  "NvEncodeAPICreateInstance");

		LOGENGINE() << TOSTRING(NVENCAPICreateInstance) << "状态"
					<< static_cast<uint32_t>(NVENCAPICreateInstance(&nvencAPI));

		NV_ENC_OPEN_ENCODE_SESSION_EX_PARAMS sessionParams = { NV_ENC_OPEN_ENCODE_SESSION_EX_PARAMS_VER };
		sessionParams.device = GraphicsDevice::get();
		sessionParams.deviceType = NV_ENC_DEVICE_TYPE_DIRECTX;
		sessionParams.apiVersion = NVENCAPI_VERSION;

		NVENCCALL(nvencAPI.nvEncOpenEncodeSessionEx(&sessionParams, &encoder));

		NV_ENC_PRESET_CONFIG presetConfig = { NV_ENC_PRESET_CONFIG_VER, { NV_ENC_CONFIG_VER } };

		NVENCCALL(nvencAPI.nvEncGetEncodePresetConfigEx(encoder, codec, preset, tuningInfo, &presetConfig));

		NV_ENC_CONFIG& config = presetConfig.presetCfg;
		config.version = NV_ENC_CONFIG_VER;
		config.profileGUID = profile;

		//高质量编码
		config.gopLength = gopLength;
		config.frameIntervalP = frameIntervalP;
		config.rcParams.enableLookahead = 1;
		config.rcParams.lookaheadDepth = lookaheadDepth;
		config.rcParams.rateControlMode = NV_ENC_PARAMS_RC_VBR;
		config.rcParams.targetQuality = 18;
		config.rcParams.averageBitRate = 0;
		config.rcParams.enableAQ = 1;
		config.rcParams.enableTemporalAQ = 1;
		config.rcParams.multiPass = NV_ENC_TWO_PASS_FULL_RESOLUTION;
		config.encodeCodecConfig.h264Config.useBFramesAsRef = NV_ENC_BFRAME_REF_MODE_EACH;

		//小于2K
		if (Graphics::getWidth() < 2048u)
		{
			config.rcParams.maxBitRate = 80000000U;
		}
		else
		{
			config.rcParams.maxBitRate = 160000000U;
		}

		config.rcParams.vbvBufferSize = config.rcParams.maxBitRate * 4;

		NV_ENC_CONFIG_H264_VUI_PARAMETERS& vuiParameters =
			config.encodeCodecConfig.h264Config.h264VUIParameters;
		vuiParameters.videoSignalTypePresentFlag = 1;
		vuiParameters.videoFormat = NV_ENC_VUI_VIDEO_FORMAT_UNSPECIFIED;
		vuiParameters.videoFullRangeFlag = 0;
		vuiParameters.colourDescriptionPresentFlag = 1;
		vuiParameters.colourPrimaries = NV_ENC_VUI_COLOR_PRIMARIES_BT709;
		vuiParameters.colourMatrix = NV_ENC_VUI_MATRIX_COEFFS_BT709;
		vuiParameters.transferCharacteristics = NV_ENC_VUI_TRANSFER_CHARACTERISTIC_BT470M;
		vuiParameters.chromaSampleLocationFlag = 1;
		vuiParameters.chromaSampleLocationTop = 0;
		vuiParameters.chromaSampleLocationBot = 0;

		NV_ENC_INITIALIZE_PARAMS encoderParams = { NV_ENC_INITIALIZE_PARAMS_VER };
		encoderParams.bufferFormat = bufferFormat;
		encoderParams.encodeConfig = &config;
		encoderParams.encodeGUID = codec;
		encoderParams.presetGUID = preset;
		encoderParams.tuningInfo = tuningInfo;
		encoderParams.encodeWidth = Graphics::getWidth();
		encoderParams.encodeHeight = Graphics::getHeight();
		encoderParams.darWidth = Graphics::getWidth();
		encoderParams.darHeight = Graphics::getHeight();
		encoderParams.maxEncodeWidth = Graphics::getWidth();
		encoderParams.maxEncodeHeight = Graphics::getHeight();
		encoderParams.frameRateNum = frameRate;
		encoderParams.frameRateDen = 1;
		encoderParams.enablePTD = 1;
		encoderParams.enableOutputInVidmem = 0;
		encoderParams.enableEncodeAsync = 1;

		NVENCCALL(nvencAPI.nvEncInitializeEncoder(encoder, &encoderParams));

		eosEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		;

		completionEvents = makeUnique<HANDLE[]>(numNV12Textures);

		outputResources = makeUnique<NV_ENC_OUTPUT_RESOURCE_D3D12[]>(numNV12Textures);

		decodeFrameIndices = makeUnique<uint64_t[]>(numNV12Textures);

		mappedInputResourcePtrs = makeUnique<NV_ENC_INPUT_PTR[]>(numNV12Textures);

		mappedOutputResourcePtrs = makeUnique<NV_ENC_INPUT_PTR[]>(numNV12Textures);

		nv12Textures = makeUnique<D3D12Resource::TexturePtr[]>(numNV12Textures);

		readbackHeaps = makeUnique<D3D12Resource::ReadbackHeapPtr[]>(numNV12Textures);

		registeredInputResourcePtrs = makeUnique<NV_ENC_REGISTERED_PTR[]>(numNV12Textures);

		registeredOutputResourcePtrs = makeUnique<NV_ENC_REGISTERED_PTR[]>(numNV12Textures);

		for (uint32_t i = 0; i < numNV12Textures; i++)
		{
			completionEvents[i] = CreateEvent(nullptr, FALSE, FALSE, nullptr);

			nv12Textures[i] = makeUnique<D3D12Resource::Texture>(Graphics::getWidth(),
			                                                     Graphics::getHeight(),
			                                                     FMT::NV12,
			                                                     1,
			                                                     1,
			                                                     true,
			                                                     D3D12_RESOURCE_FLAG_NONE,
			                                                     nullptr,
			                                                     D3D12_RESOURCE_STATE_COMMON);

			readbackHeaps[i] = makeUnique<D3D12Resource::ReadbackHeap>(readbackHeapSize);

			NV_ENC_REGISTER_RESOURCE registerInputResource = { NV_ENC_REGISTER_RESOURCE_VER };
			registerInputResource.bufferFormat = bufferFormat;
			registerInputResource.bufferUsage = NV_ENC_INPUT_IMAGE;
			registerInputResource.resourceType = NV_ENC_INPUT_RESOURCE_TYPE_DIRECTX;
			registerInputResource.resourceToRegister = nv12Textures[i]->getResource();
			registerInputResource.subResourceIndex = 0;
			registerInputResource.width = Graphics::getWidth();
			registerInputResource.height = Graphics::getHeight();
			registerInputResource.pitch = 0;
			registerInputResource.pInputFencePoint = nullptr;

			NVENCCALL(nvencAPI.nvEncRegisterResource(encoder, &registerInputResource));

			registeredInputResourcePtrs[i] = registerInputResource.registeredResource;

			NV_ENC_REGISTER_RESOURCE registerOutputResource = { NV_ENC_REGISTER_RESOURCE_VER };
			registerOutputResource.bufferFormat = NV_ENC_BUFFER_FORMAT_U8;
			registerOutputResource.bufferUsage = NV_ENC_OUTPUT_BITSTREAM;
			registerOutputResource.resourceType = NV_ENC_INPUT_RESOURCE_TYPE_DIRECTX;
			registerOutputResource.resourceToRegister = readbackHeaps[i]->getResource();
			registerOutputResource.subResourceIndex = 0;
			registerOutputResource.width = static_cast<uint32_t>(readbackHeapSize);
			registerOutputResource.height = 1;
			registerOutputResource.pitch = 0;
			registerOutputResource.pInputFencePoint = nullptr;

			NVENCCALL(nvencAPI.nvEncRegisterResource(encoder, &registerOutputResource));

			registeredOutputResourcePtrs[i] = registerOutputResource.registeredResource;
		}

		writeHeader();

		workerThread = std::thread(&NVIDIAEncoder::workerLoop, this);
	}

	NVIDIAEncoder::~NVIDIAEncoder()
	{
		if (moduleNvEncAPI)
		{
			if (workerThread.joinable())
			{
				workerThread.join();
			}

			//发送EOS结束编码
			{
				NV_ENC_PIC_PARAMS eosParams = { NV_ENC_PIC_PARAMS_VER };

				eosParams.encodePicFlags = NV_ENC_PIC_FLAG_EOS;

				eosParams.completionEvent = eosEvent;

				nvencAPI.nvEncEncodePicture(encoder, &eosParams);

				WaitForSingleObject(eosEvent, INFINITE);
			}

			//编码结束后进行冲刷
			while (readIndex < writeIndex)
			{
				const uint64_t modReadIndex = readIndex % numNV12Textures;

				WaitForSingleObject(completionEvents[modReadIndex], INFINITE);

				NV_ENC_LOCK_BITSTREAM lockBitstream = { NV_ENC_LOCK_BITSTREAM_VER };

				lockBitstream.outputBitstream = &outputResources[modReadIndex];

				lockBitstream.doNotWait = 0;

				const NVENCSTATUS status = nvencAPI.nvEncLockBitstream(encoder, &lockBitstream);

				if (status != NV_ENC_SUCCESS)
				{
					nvencAPI.nvEncUnmapInputResource(encoder, mappedInputResourcePtrs[modReadIndex]);

					nvencAPI.nvEncUnmapInputResource(encoder, mappedOutputResourcePtrs[modReadIndex]);

					readIndex++;

					break;
				}

				nvencAPI.nvEncUnlockBitstream(encoder, lockBitstream.outputBitstream);

				nvencAPI.nvEncUnmapInputResource(encoder, mappedInputResourcePtrs[modReadIndex]);

				nvencAPI.nvEncUnmapInputResource(encoder, mappedOutputResourcePtrs[modReadIndex]);

				readIndex++;
			}

			while (readIndex < writeIndex)
			{
				const uint64_t modReadIndex = readIndex % numNV12Textures;

				WaitForSingleObject(completionEvents[modReadIndex], INFINITE);

				nvencAPI.nvEncUnmapInputResource(encoder, mappedInputResourcePtrs[modReadIndex]);

				nvencAPI.nvEncUnmapInputResource(encoder, mappedOutputResourcePtrs[modReadIndex]);

				readIndex++;
			}

			for (uint32_t i = 0; i < numNV12Textures; i++)
			{
				nvencAPI.nvEncUnregisterResource(encoder, registeredOutputResourcePtrs[i]);

				nvencAPI.nvEncUnregisterResource(encoder, registeredInputResourcePtrs[i]);
			}

			for (uint32_t i = 0; i < numNV12Textures; i++)
			{
				CloseHandle(completionEvents[i]);
			}

			CloseHandle(eosEvent);

			nvencAPI.nvEncDestroyEncoder(encoder);

			FreeLibrary(moduleNvEncAPI);
		}
	}

	bool NVIDIAEncoder::encode(D3D12Resource::Texture* const inputTexture)
	{
		if (writeIndex >= readIndex + numNV12Textures)
		{
			std::unique_lock<std::mutex> readIndexLock(readIndexMutex);

			readCV.wait(readIndexLock,
			            [this]
			{
				return writeIndex < readIndex + numNV12Textures;
			});
		}

		const uint64_t modWriteIndex = writeIndex % numNV12Textures;

		const uint64_t inputFenceWaitValue =
			bgraToNV12(inputTexture, nv12Textures[modWriteIndex].get(), inputFence.get());

		NV_ENC_MAP_INPUT_RESOURCE mapInputResource = { NV_ENC_MAP_INPUT_RESOURCE_VER };
		mapInputResource.registeredResource = registeredInputResourcePtrs[modWriteIndex];

		NVENCCALL(nvencAPI.nvEncMapInputResource(encoder, &mapInputResource));

		mappedInputResourcePtrs[modWriteIndex] = mapInputResource.mappedResource;

		NV_ENC_MAP_INPUT_RESOURCE mapOutputResource = { NV_ENC_MAP_INPUT_RESOURCE_VER };
		mapOutputResource.registeredResource = registeredOutputResourcePtrs[modWriteIndex];

		NVENCCALL(nvencAPI.nvEncMapInputResource(encoder, &mapOutputResource));

		mappedOutputResourcePtrs[modWriteIndex] = mapOutputResource.mappedResource;

		NV_ENC_INPUT_RESOURCE_D3D12 inputResource = { NV_ENC_INPUT_RESOURCE_D3D12_VER };
		inputResource.pInputBuffer = mapInputResource.mappedResource;
		inputResource.inputFencePoint = NV_ENC_FENCE_POINT_D3D12{ NV_ENC_FENCE_POINT_D3D12_VER };
		inputResource.inputFencePoint.pFence = inputFence->get();
		inputResource.inputFencePoint.waitValue = inputFenceWaitValue;
		inputResource.inputFencePoint.bWait = true;

		NV_ENC_OUTPUT_RESOURCE_D3D12 outputResource = { NV_ENC_OUTPUT_RESOURCE_D3D12_VER };
		outputResource.pOutputBuffer = mapOutputResource.mappedResource;
		outputResource.outputFencePoint = NV_ENC_FENCE_POINT_D3D12{ NV_ENC_FENCE_POINT_D3D12_VER };
		outputResource.outputFencePoint.pFence = outputFence->get();
		outputResource.outputFencePoint.signalValue = outputFence->increment();
		outputResource.outputFencePoint.bSignal = true;

		outputResources[modWriteIndex] = outputResource;

		NV_ENC_PIC_PARAMS picParams = { NV_ENC_PIC_PARAMS_VER };

		picParams.pictureStruct = NV_ENC_PIC_STRUCT_FRAME;

		picParams.inputBuffer = &inputResource;

		picParams.outputBitstream = &outputResource;

		picParams.bufferFmt = bufferFormat;

		picParams.inputWidth = Graphics::getWidth();

		picParams.inputHeight = Graphics::getHeight();

		picParams.completionEvent = completionEvents[modWriteIndex];

		picParams.frameIdx = static_cast<uint32_t>(Graphics::getRenderedFrameCount());

		picParams.inputTimeStamp = Graphics::getRenderedFrameCount();

		decodeFrameIndices[modWriteIndex] = Graphics::getRenderedFrameCount();

		NVENCCALL(nvencAPI.nvEncEncodePicture(encoder, &picParams));

		{
			std::lock_guard<std::mutex> writeIndexLock(writeIndexMutex);

			writeIndex++;
		}

		writeCV.notify_one();

		return encoding;
	}

	void NVIDIAEncoder::workerLoop()
	{
		while (true)
		{
			if (readIndex >= writeIndex)
			{
				std::unique_lock<std::mutex> writeIndexLock(writeIndexMutex);

				writeCV.wait(writeIndexLock,
				             [this]
				{
					return readIndex < writeIndex;
				});
			}

			const uint64_t modReadIndex = readIndex % numNV12Textures;

			WaitForSingleObject(completionEvents[modReadIndex], INFINITE);

			NV_ENC_LOCK_BITSTREAM lockBitstream = { NV_ENC_LOCK_BITSTREAM_VER };

			lockBitstream.outputBitstream = &outputResources[modReadIndex];

			lockBitstream.doNotWait = 0;

			NVENCCALL(nvencAPI.nvEncLockBitstream(encoder, &lockBitstream));

			//[mp4 @ 000001bdb1381100] pts (46500) < dts (48000) in stream 0 报错
			//解决方法 https://github.com/FFmpeg/FFmpeg/commit/670ff6c7ce0c70798a9909b334310625fe067a34?diff=split
			encoding = writeFrame(lockBitstream.bitstreamBufferPtr,
			                      lockBitstream.bitstreamSizeInBytes,
			                      lockBitstream.pictureType == NV_ENC_PIC_TYPE_IDR,
			                      static_cast<int64_t>(decodeFrameIndices[modReadIndex]) -
			                          static_cast<int64_t>(frameIntervalP - 1u),
			                      static_cast<int64_t>(lockBitstream.outputTimeStamp));

			NVENCCALL(nvencAPI.nvEncUnlockBitstream(encoder, lockBitstream.outputBitstream));

			NVENCCALL(nvencAPI.nvEncUnmapInputResource(encoder, mappedInputResourcePtrs[modReadIndex]));

			NVENCCALL(nvencAPI.nvEncUnmapInputResource(encoder, mappedOutputResourcePtrs[modReadIndex]));

			{
				std::lock_guard<std::mutex> readIndexLock(readIndexMutex);

				readIndex++;
			}

			readCV.notify_one();

			if (!encoding)
			{
				return;
			}
		}
	}
}
