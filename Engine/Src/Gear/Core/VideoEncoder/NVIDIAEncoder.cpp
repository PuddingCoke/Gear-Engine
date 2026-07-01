#include<Gear/Core/VideoEncoder/NVIDIAEncoder.h>

#include<iostream>

#define NVENCCALL(func) \
{\
const NVENCSTATUS status = func;\
if(status != NV_ENC_SUCCESS && status != NV_ENC_ERR_NEED_MORE_INPUT)\
{\
std::cout<<"error occured at function "<<#func<<"\n";\
const char* error = nvencAPI.nvEncGetLastErrorString(encoder);\
std::cout << "status " << status << "\n";\
std::cout << error << "\n";\
__debugbreak();\
}\
}\

namespace Gear::Core::VideoEncoder
{
	NVIDIAEncoder::NVIDIAEncoder(const uint32_t frameToEncode) :
		Encoder(frameToEncode, outputVideoFormat), encoder(nullptr),
		readbackHeap(makeUnique<D3D12Resource::ReadbackHeap>(2 * 4 * Graphics::getWidth() * Graphics::getHeight())),
		nvencAPI{ NV_ENCODE_API_FUNCTION_LIST_VER },
		outputFenceValue(0)
	{
		moduleNvEncAPI = LoadLibraryA("nvEncodeAPI64.dll");

		if (moduleNvEncAPI == 0)
		{
			LOGERROR(L"无法读取nvEncodeAPI64.dll！");
		}

		NVENCSTATUS(__stdcall * NVENCAPICreateInstance)(NV_ENCODE_API_FUNCTION_LIST*) = (NVENCSTATUS(*)(NV_ENCODE_API_FUNCTION_LIST*))GetProcAddress(moduleNvEncAPI, "NvEncodeAPICreateInstance");

		LOGENGINE(TOWSTRING(NVENCAPICreateInstance), L"状态", static_cast<uint32_t>(NVENCAPICreateInstance(&nvencAPI)));

		NV_ENC_OPEN_ENCODE_SESSION_EX_PARAMS sessionParams = { NV_ENC_OPEN_ENCODE_SESSION_EX_PARAMS_VER };
		sessionParams.device = GraphicsDevice::get();
		sessionParams.deviceType = NV_ENC_DEVICE_TYPE_DIRECTX;
		sessionParams.apiVersion = NVENCAPI_VERSION;

		NVENCCALL(nvencAPI.nvEncOpenEncodeSessionEx(&sessionParams, &encoder));

		NV_ENC_PRESET_CONFIG presetConfig = { NV_ENC_PRESET_CONFIG_VER,{NV_ENC_CONFIG_VER} };

		NVENCCALL(nvencAPI.nvEncGetEncodePresetConfigEx(encoder, codec, preset, tuningInfo, &presetConfig));

		NV_ENC_CONFIG config;
		memcpy(&config, &presetConfig.presetCfg, sizeof(NV_ENC_CONFIG));
		config.version = NV_ENC_CONFIG_VER;
		config.profileGUID = profile;

		//高质量编码
		config.gopLength = 120;
		config.frameIntervalP = 1;
		config.rcParams.enableLookahead = 1;
		config.rcParams.lookaheadDepth = lookaheadDepth;
		config.rcParams.rateControlMode = NV_ENC_PARAMS_RC_VBR;
		config.rcParams.vbvBufferSize = config.rcParams.maxBitRate * 8;
		config.rcParams.enableAQ = 1;
		config.rcParams.multiPass = NV_ENC_TWO_PASS_FULL_RESOLUTION;

		//小于2K
		if (Graphics::getWidth() < 2048u)
		{
			config.rcParams.averageBitRate = 40000000U;
			config.rcParams.maxBitRate = 80000000U;
		}
		else
		{
			config.rcParams.averageBitRate = 80000000U;
			config.rcParams.maxBitRate = 160000000U;
		}

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
		encoderParams.enableEncodeAsync = 0;

		NVENCCALL(nvencAPI.nvEncInitializeEncoder(encoder, &encoderParams));

		GraphicsDevice::get()->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&outputFence));

		LOGENGINE(L"开始编码");

		NV_ENC_REGISTER_RESOURCE registerOutputResource = { NV_ENC_REGISTER_RESOURCE_VER };
		registerOutputResource.bufferFormat = NV_ENC_BUFFER_FORMAT_U8;
		registerOutputResource.bufferUsage = NV_ENC_OUTPUT_BITSTREAM;
		registerOutputResource.resourceType = NV_ENC_INPUT_RESOURCE_TYPE_DIRECTX;
		registerOutputResource.resourceToRegister = readbackHeap->getResource();
		registerOutputResource.subResourceIndex = 0;
		registerOutputResource.width = 2 * 4 * Graphics::getWidth() * Graphics::getHeight();
		registerOutputResource.height = 1;
		registerOutputResource.pitch = 0;
		registerOutputResource.pInputFencePoint = nullptr;

		NVENCCALL(nvencAPI.nvEncRegisterResource(encoder, &registerOutputResource));

		registeredOutputResourcePtr = registerOutputResource.registeredResource;

		NV_ENC_MAP_INPUT_RESOURCE mapOutputResource = { NV_ENC_MAP_INPUT_RESOURCE_VER };
		mapOutputResource.registeredResource = registerOutputResource.registeredResource;

		NVENCCALL(nvencAPI.nvEncMapInputResource(encoder, &mapOutputResource));

		mappedOutputResourcePtr = mapOutputResource.mappedResource;
	}

	NVIDIAEncoder::~NVIDIAEncoder()
	{
		if (moduleNvEncAPI)
		{
			nvencAPI.nvEncUnmapInputResource(encoder, mappedOutputResourcePtr);

			nvencAPI.nvEncUnregisterResource(encoder, registeredOutputResourcePtr);

			while (mappedInputResourcePtrs.size())
			{
				nvencAPI.nvEncUnmapInputResource(encoder, mappedInputResourcePtrs.front());

				mappedInputResourcePtrs.pop();
			}

			while (registeredInputResourcePtrs.size())
			{
				nvencAPI.nvEncUnregisterResource(encoder, registeredInputResourcePtrs.front());

				registeredInputResourcePtrs.pop();
			}

			NVENCCALL(nvencAPI.nvEncDestroyEncoder(encoder));

			FreeLibrary(moduleNvEncAPI);
		}
	}

	bool NVIDIAEncoder::encode(D3D12Resource::Texture* const inputTexture)
	{
		bool encoding = true;

		NV_ENC_REGISTER_RESOURCE registerInputResource = { NV_ENC_REGISTER_RESOURCE_VER };
		registerInputResource.bufferFormat = bufferFormat;
		registerInputResource.bufferUsage = NV_ENC_INPUT_IMAGE;
		registerInputResource.resourceType = NV_ENC_INPUT_RESOURCE_TYPE_DIRECTX;
		registerInputResource.resourceToRegister = inputTexture->getResource();
		registerInputResource.subResourceIndex = 0;
		registerInputResource.width = Graphics::getWidth();
		registerInputResource.height = Graphics::getHeight();
		registerInputResource.pitch = 0;
		registerInputResource.pInputFencePoint = nullptr;

		NVENCCALL(nvencAPI.nvEncRegisterResource(encoder, &registerInputResource));

		registeredInputResourcePtrs.push(registerInputResource.registeredResource);

		NV_ENC_MAP_INPUT_RESOURCE mapInputResource = { NV_ENC_MAP_INPUT_RESOURCE_VER };
		mapInputResource.registeredResource = registerInputResource.registeredResource;

		NVENCCALL(nvencAPI.nvEncMapInputResource(encoder, &mapInputResource));

		mappedInputResourcePtrs.push(mapInputResource.mappedResource);

		NV_ENC_INPUT_RESOURCE_D3D12 inputResource = { NV_ENC_INPUT_RESOURCE_D3D12_VER };
		inputResource.pInputBuffer = mapInputResource.mappedResource;
		inputResource.inputFencePoint = NV_ENC_FENCE_POINT_D3D12{ NV_ENC_FENCE_POINT_D3D12_VER };

		NV_ENC_OUTPUT_RESOURCE_D3D12 outputResource = { NV_ENC_INPUT_RESOURCE_D3D12_VER };
		outputResource.pOutputBuffer = mappedOutputResourcePtr;
		outputResource.outputFencePoint = NV_ENC_FENCE_POINT_D3D12{ NV_ENC_FENCE_POINT_D3D12_VER };
		outputResource.outputFencePoint.pFence = outputFence.Get();
		outputResource.outputFencePoint.signalValue = ++outputFenceValue;
		outputResource.outputFencePoint.bSignal = true;

		outputResources.push(outputResource);

		NV_ENC_PIC_PARAMS picParams = { NV_ENC_PIC_PARAMS_VER };

		picParams.pictureStruct = NV_ENC_PIC_STRUCT_FRAME;

		picParams.inputBuffer = &inputResource;

		picParams.outputBitstream = &outputResource;

		picParams.bufferFmt = bufferFormat;

		picParams.inputWidth = Graphics::getWidth();

		picParams.inputHeight = Graphics::getHeight();

		picParams.completionEvent = nullptr;

		const NVENCSTATUS status = nvencAPI.nvEncEncodePicture(encoder, &picParams);

		if (status == NV_ENC_SUCCESS)
		{
			NV_ENC_LOCK_BITSTREAM lockBitstream = { NV_ENC_LOCK_BITSTREAM_VER };

			lockBitstream.outputBitstream = &outputResources.front();

			lockBitstream.doNotWait = 0;

			NVENCCALL(nvencAPI.nvEncLockBitstream(encoder, &lockBitstream));

			encoding = writeFrame(lockBitstream.bitstreamBufferPtr, lockBitstream.bitstreamSizeInBytes, lockBitstream.pictureType == NV_ENC_PIC_TYPE_IDR);

			NVENCCALL(nvencAPI.nvEncUnlockBitstream(encoder, lockBitstream.outputBitstream));

			outputResources.pop();

			nvencAPI.nvEncUnmapInputResource(encoder, mappedInputResourcePtrs.front());

			mappedInputResourcePtrs.pop();

			nvencAPI.nvEncUnregisterResource(encoder, registeredInputResourcePtrs.front());

			registeredInputResourcePtrs.pop();

			if (!encoding)
			{
				NV_ENC_PIC_PARAMS eosParams = { NV_ENC_PIC_PARAMS_VER };

				eosParams.encodePicFlags = NV_ENC_PIC_FLAG_EOS;

				NVENCCALL(nvencAPI.nvEncEncodePicture(encoder, &eosParams));
			}
		}
		else if (status != NV_ENC_ERR_NEED_MORE_INPUT)
		{
			const char* error = nvencAPI.nvEncGetLastErrorString(encoder);

			std::cout << "status " << status << "\n";

			std::cout << error << "\n";

			__debugbreak();
		}

		return encoding;
	}
}
