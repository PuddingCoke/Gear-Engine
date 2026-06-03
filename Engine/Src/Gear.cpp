#include<Gear/Gear.h>

#include<Gear/Window/Win32Form.h>

#include<Gear/Core/RenderEngine.h>

#include<Gear/Core/Internal/RenderEngineInternal.h>

#include<Gear/Utils/WallpaperHelper.h>

#include<Gear/Core/Graphics.h>

#include<Gear/Input/Keyboard.h>

#include<Gear/Utils/Math.h>

#include<Gear/Utils/File.h>

#include<Gear/Utils/Logger.h>

#include<Gear/Core/VideoEncoder/NvidiaEncoder.h>

#include<Gear/Utils/Internal/LoggerInternal.h>

#include<Gear/Utils/Internal/FileInternal.h>

#include<dxgidebug.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION

#include<stb_image_write.h>

namespace Gear
{
	class GearImpl
	{
	public:

		GearImpl(const GearImpl&) = delete;

		void operator=(const GearImpl&) = delete;

		GearImpl();

		~GearImpl();

		int32_t iniEngine(const InitializationParam& param, const int32_t argc, const wchar_t* argv[]);

		void iniGame(UniquePtr<Game> gamePtr);

	private:

		void runRealTimeRender();

		void runVideoRender();

		void runWallpaper();

		void reportLiveObjects() const;

		UniquePtr<Utils::Logger::Internal::InitializeToken> loggerToken;

		UniquePtr<Window::Win32Form::InitializeToken> windowToken;

		UniquePtr<Core::RenderEngine::Internal::InitializeToken> renderEngineToken;

		UniquePtr<Game> game;

		//用于截屏
		UniquePtr<Core::Resource::D3D12Resource::ReadbackHeap> backBufferHeap;

		InitializationParam::EngineUsage usage;

		union
		{
			InitializationParam::RealTimeRenderParam realTimeRender;

			InitializationParam::VideoRenderParam videoRender;
		};

		static constexpr Input::Keyboard::Key screenGrabKey = Input::Keyboard::F11;

	};

	GearImpl::GearImpl()
	{
	}

	GearImpl::~GearImpl()
	{
		LOGENGINE(L"destroy resources");

		Core::RenderEngine::Internal::waitForCurrentFrame();

#ifdef _DEBUG

		//由于Debug模式下需要输出存活的对象，所以需要手动释放相关资源
		backBufferHeap.reset();

		game.reset();

		renderEngineToken.reset();

		reportLiveObjects();

#endif // _DEBUG
	}

	int32_t GearImpl::iniEngine(const InitializationParam& param, const int32_t argc, const wchar_t* argv[])
	{
		loggerToken = makeUnique<Utils::Logger::Internal::InitializeToken>();

		Utils::File::Internal::setRootFolder(Utils::File::backslashToSlash(Utils::File::getParentFolder(argv[0])));

		LOGENGINE(L"root folder", LogColor::brightBlue, Utils::File::getRootFolder());

		usage = param.usage;

		DirectX::XMUINT2 systemResolution = {};

		Utils::WallpaperHelper::getSystemResolution(systemResolution.x, systemResolution.y);

		switch (usage)
		{
		case InitializationParam::EngineUsage::REALTIMERENDER:

			realTimeRender = param.realTimeRender;

			windowToken = makeUnique<Window::Win32Form::InitializeToken>(param.title, (systemResolution.x - realTimeRender.width) / 2, (systemResolution.y - realTimeRender.height) / 2,
				realTimeRender.width, realTimeRender.height, Window::Win32Form::normalWindowStyle, Window::Win32Form::windowCallback);

			renderEngineToken = makeUnique<Core::RenderEngine::Internal::InitializeToken>(realTimeRender.width, realTimeRender.height, Window::Win32Form::getHandle(), true, realTimeRender.enableImGuiSurface);

			backBufferHeap = makeUnique<Core::Resource::D3D12Resource::ReadbackHeap>(Core::FMT::getByteSize(Core::Graphics::backBufferFormat) * realTimeRender.width * realTimeRender.height);

			LOGENGINE(L"engine usage real time render");

			break;

		case InitializationParam::EngineUsage::VIDEORENDER:

			videoRender = param.videoRender;

			windowToken = makeUnique<Window::Win32Form::InitializeToken>(param.title, 100, 100, 100, 100, Window::Win32Form::normalWindowStyle, Window::Win32Form::encodeCallback);

			ShowWindow(Window::Win32Form::getHandle(), SW_HIDE);

			renderEngineToken = makeUnique<Core::RenderEngine::Internal::InitializeToken>(videoRender.width, videoRender.height, Window::Win32Form::getHandle(), false, false);

			LOGENGINE(L"engine usage video render");

			break;

		case InitializationParam::EngineUsage::WALLPAPER:

			windowToken = makeUnique<Window::Win32Form::InitializeToken>(param.title, 0, 0, systemResolution.x, systemResolution.y, Window::Win32Form::wallpaperWindowStyle, Window::Win32Form::wallpaperCallBack);

			{
				const HWND parentHWND = Utils::WallpaperHelper::getWallpaperHWND();

				SetParent(Window::Win32Form::getHandle(), parentHWND);
			}

			renderEngineToken = makeUnique<Core::RenderEngine::Internal::InitializeToken>(systemResolution.x, systemResolution.y, Window::Win32Form::getHandle(), true, false);

			LOGENGINE(L"engine usage wallpaper");

			break;

		default:
			break;
		}

		LOGENGINE(L"resolution", Core::Graphics::getWidth(), L"x", Core::Graphics::getHeight());

		LOGENGINE(L"aspect ratio", Core::Graphics::getAspectRatio());

		LOGENGINE(L"back buffer count", Core::Graphics::getFrameBufferCount());

		return 0;
	}

	void GearImpl::iniGame(UniquePtr<Game> gamePtr)
	{
		game = std::move(gamePtr);

		Core::RenderEngine::Internal::initializeResources();

		switch (usage)
		{
		case InitializationParam::EngineUsage::REALTIMERENDER:
			runRealTimeRender();
			break;

		case InitializationParam::EngineUsage::VIDEORENDER:
			runVideoRender();
			break;

		case InitializationParam::EngineUsage::WALLPAPER:
			runWallpaper();
			break;

		default:
			break;
		}
	}

	void GearImpl::runRealTimeRender()
	{
		Utils::DeltaTimeEstimator dtEstimator;

		while (Window::Win32Form::pollEvents())
		{
			const std::chrono::high_resolution_clock::time_point startPoint = std::chrono::high_resolution_clock::now();

			Core::RenderEngine::Internal::setDefRenderTexture();

			Core::RenderEngine::Internal::begin();

			game->update(Core::Graphics::getDeltaTime());

			game->render();

			const bool needScreenGrab = Input::Keyboard::onKeyDown(screenGrabKey);

			if (needScreenGrab)
			{
				Core::RenderEngine::Internal::saveBackBuffer(backBufferHeap.get());
			}

			Core::RenderEngine::Internal::end();

			Core::RenderEngine::Internal::present();

			if (needScreenGrab)
			{
				Core::RenderEngine::Internal::waitForCurrentFrame();
			}

			Core::RenderEngine::Internal::waitForNextFrame();

			const std::chrono::high_resolution_clock::time_point endPoint = std::chrono::high_resolution_clock::now();

			const float deltaTime = std::chrono::duration<float>(endPoint - startPoint).count();

			const float lerpDeltaTime = dtEstimator.getDeltaTime(deltaTime);

			Core::RenderEngine::Internal::setDeltaTime(lerpDeltaTime);

			Core::RenderEngine::Internal::updateTimeElapsed();

			if (needScreenGrab)
			{
				const uint8_t* const dataPtr = reinterpret_cast<uint8_t*>(backBufferHeap->map(CD3DX12_RANGE(0ull,
					Core::FMT::getByteSize(Core::Graphics::backBufferFormat) * realTimeRender.width * realTimeRender.height)));

				uint8_t* const colors = new uint8_t[Core::FMT::getByteSize(Core::Graphics::backBufferFormat) * realTimeRender.width * realTimeRender.height];

				for (uint32_t i = 0; i < realTimeRender.width * realTimeRender.height; i++)
				{
					const uint32_t pixel = 4 * i;

					//RGBA <- BGRA
					colors[pixel] = dataPtr[pixel + 2];

					colors[pixel + 1] = dataPtr[pixel + 1];

					colors[pixel + 2] = dataPtr[pixel];

					colors[pixel + 3] = 0xFFu;
				}

				backBufferHeap->unmap();

				stbi_write_png("output.png", realTimeRender.width, realTimeRender.height, 4, colors, Core::FMT::getByteSize(Core::Graphics::backBufferFormat) * realTimeRender.width);

				delete[] colors;

				LOGSUCCESS(L"screenshot saved to output.png");
			}
		}
	}

	void GearImpl::runVideoRender()
	{
		const Core::GPUVendor vendor = Core::RenderEngine::getVendor();

		Core::VideoEncoder::Encoder* encoder = nullptr;

		const uint32_t frameToEncode = videoRender.second * Core::VideoEncoder::Encoder::frameRate;

		switch (vendor)
		{
		case Core::GPUVendor::NVIDIA:
			encoder = new Core::VideoEncoder::NvidiaEncoder(frameToEncode);
			break;
		case Core::GPUVendor::AMD:
		case Core::GPUVendor::INTEL:
		case Core::GPUVendor::UNKNOWN:
			break;
		default:
			break;
		}

		if (vendor == Core::GPUVendor::NVIDIA)
		{
			const uint32_t numTextures = Core::VideoEncoder::NvidiaEncoder::lookaheadDepth + 1;

			Core::Resource::D3D12Resource::Texture* renderTextures[numTextures] = {};

			D3D12_CPU_DESCRIPTOR_HANDLE textureHandles[numTextures] = {};

			{
				Core::D3D12Core::DescriptorHandle descriptorHandle = Core::LocalDescriptorHeap::getRenderTargetHeap()->allocStaticDescriptor(numTextures);

				D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
				rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
				rtvDesc.Format = Core::Graphics::backBufferFormat;
				rtvDesc.Texture2D.MipSlice = 0;
				rtvDesc.Texture2D.PlaneSlice = 0;

				for (uint32_t i = 0; i < numTextures; i++)
				{
					const D3D12_CLEAR_VALUE clearValue = { Core::Graphics::backBufferFormat ,{0.f,0.f,0.f,1.f} };

					renderTextures[i] = new Core::Resource::D3D12Resource::Texture(Core::Graphics::getWidth(), Core::Graphics::getHeight(), Core::Graphics::backBufferFormat, 1, 1, true, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, &clearValue);

					Core::GraphicsDevice::get()->CreateRenderTargetView(renderTextures[i]->getResource(), &rtvDesc, descriptorHandle.getCurrentCPUHandle());

					textureHandles[i] = descriptorHandle.getCurrentCPUHandle();

					descriptorHandle.move();
				}
			}

			uint32_t index = 0;

			Core::RenderEngine::Internal::setDeltaTime(1.f / static_cast<float>(Core::VideoEncoder::Encoder::frameRate));

			while (true)
			{
				Core::RenderEngine::Internal::setRenderTexture(renderTextures[index], textureHandles[index]);

				Core::RenderEngine::Internal::begin();

				game->update(Core::Graphics::getDeltaTime());

				game->render();

				Core::RenderEngine::Internal::end();

				Core::RenderEngine::Internal::waitForCurrentFrame();

				Core::RenderEngine::Internal::updateTimeElapsed();

				if (!encoder->encode(Core::RenderEngine::getRenderTexture()))
				{
					break;
				}

				index = (index + 1) % numTextures;
			}

			for (uint32_t i = 0; i < numTextures; i++)
			{
				delete renderTextures[i];
			}
		}

		if (encoder)
		{
			delete encoder;
		}
	}

	void GearImpl::runWallpaper()
	{
		Utils::DeltaTimeEstimator dtEstimator;

		while (Window::Win32Form::pollEvents())
		{
			const std::chrono::high_resolution_clock::time_point startPoint = std::chrono::high_resolution_clock::now();

			Core::RenderEngine::Internal::setDefRenderTexture();

			Core::RenderEngine::Internal::begin();

			game->update(Core::Graphics::getDeltaTime());

			game->render();

			Core::RenderEngine::Internal::end();

			Core::RenderEngine::Internal::present();

			Core::RenderEngine::Internal::waitForNextFrame();

			const std::chrono::high_resolution_clock::time_point endPoint = std::chrono::high_resolution_clock::now();

			const float deltaTime = std::chrono::duration<float>(endPoint - startPoint).count();

			const float lerpDeltaTime = dtEstimator.getDeltaTime(deltaTime);

			Core::RenderEngine::Internal::setDeltaTime(lerpDeltaTime);

			Core::RenderEngine::Internal::updateTimeElapsed();
		}
	}

	void GearImpl::reportLiveObjects() const
	{
		ComPtr<IDXGIDebug1> dxgiDebug;

		OutputDebugStringA("**********Live Object Report**********\n");

		if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiDebug))))
		{
			dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_FLAGS(DXGI_DEBUG_RLO_ALL | DXGI_DEBUG_RLO_IGNORE_INTERNAL));
		}

		OutputDebugStringA("**********Live Object Report**********\n");
	}

	UniquePtr<GearImpl> impl;

	int32_t iniEngine(const InitializationParam& param, const int32_t argc, const wchar_t* argv[])
	{
		return impl->iniEngine(param, argc, argv);
	}

	void iniGame(UniquePtr<Game> gamePtr)
	{
		impl->iniGame(std::move(gamePtr));
	}

	void initialize()
	{
		impl = makeUnique<GearImpl>();
	}

	void release()
	{
		impl.reset();
	}

	void failureExit()
	{
		Utils::Logger::Internal::release();

		std::quick_exit(EXIT_FAILURE);
	}
}
