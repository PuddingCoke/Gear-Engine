#include<Gear/Gear.h>

#include<Gear/Window/Win32Form.h>

#include<Gear/Core/RenderEngine.h>

#include<Gear/Core/Internal/RenderEngineInternal.h>

#include<Gear/Core/Graphics.h>

#include<Gear/Input/Keyboard.h>

#include<Gear/Utils/MainMonitor.h>

#include<Gear/Utils/Internal/MainMonitorInternal.h>

#include<Gear/Utils/File.h>

#include<Gear/Utils/Internal/FileInternal.h>

#include<Gear/Utils/Internal/LoggerInternal.h>

#include<Gear/Utils/WallpaperHelper.h>

#include<Gear/Utils/DeltaTimeEstimator.h>

#include<Gear/Core/VideoEncoder/NVIDIAEncoder.h>

#include<Gear/Core/VideoEncoder/SoftwareEncoder.h>

#include<iostream>

#include<locale>

#include<chrono>

#include<dxgidebug.h>

#include<dxgi1_6.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION

#include<stb_image_write.h>

namespace
{
	std::wstring getExecutablePathW()
	{
		std::wstring buffer(MAX_PATH, L'\0');

		while (true)
		{
			const DWORD len = GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));

			if (len == 0)
			{
				return L"";
			}

			if (len == buffer.size())
			{
				break;
			}
			else if (len < buffer.size())
			{
				buffer.resize(len);

				break;
			}

			if (len > buffer.size())
			{
				buffer.resize(buffer.size() * 2);
			}
		}

		return buffer;
	}
}

namespace Gear
{
	class GearImpl
	{
	public:

		GearImpl(const GearImpl&) = delete;

		void operator=(const GearImpl&) = delete;

		GearImpl(const InitializationParam& param, const int32_t argc, const wchar_t* argv[]);

		~GearImpl();

		void initGame(UniquePtr<Game> gamePtr);

	private:

		void runRealTimeRender();

		void runVideoRender();

		void runWallpaper();

		void reportLiveObjects() const;

		UniquePtr<Logger::Internal::InitializeToken> loggerToken;

		UniquePtr<File::Internal::InitializeToken> fileToken;

		UniquePtr<Window::Win32Form::InitializeToken> windowToken;

		UniquePtr<RenderEngine::Internal::InitializeToken> renderEngineToken;

		UniquePtr<Game> game;

		D3D12_PLACED_SUBRESOURCE_FOOTPRINT backBufferFootprint;

		//用于截屏
		UniquePtr<D3D12Resource::ReadbackHeap> backBufferHeap;

		InitializationParam initParam;

		static constexpr Input::Keyboard::Key screenGrabKey = Input::Keyboard::F11;

	};

	GearImpl::GearImpl(const InitializationParam& param, const int32_t argc, const wchar_t* argv[]) :
		backBufferFootprint{}, initParam(param)
	{
		//设置locale为.UTF-8用于多语言支持
		std::locale::global(std::locale(".UTF-8"));

		loggerToken = makeUnique<Logger::Internal::InitializeToken>();

		const std::wstring executablePath = getExecutablePathW();

		if (executablePath == L"")
		{
			THROWLOG(LOGERROR() << "获取EXE路径失败！");
		}

		fileToken = makeUnique<File::Internal::InitializeToken>(File::backslashToSlash(File::getParentFolder(executablePath)));

		LOGENGINE() << "EXE根目录" << COLORIZEPATH(File::getRootFolderW());

		MainMonitor::Internal::getCurrentSettings();

		if (initParam.usage == InitializationParam::VIDEORENDER || initParam.usage == InitializationParam::WALLPAPER)
		{
			initParam.enableImGuiSurface = false;
		}

		uint32_t windowStartX = 0u;

		uint32_t windowStartY = 0u;

		uint32_t windowWidth = 0u;

		uint32_t windowHeight = 0u;

		DWORD windowStyle = 0u;

		LRESULT(*windowCallback)(HWND hWnd, uint32_t msg, WPARAM wParam, LPARAM lParam) = nullptr;

		//根据初始化参数配置窗口的初始化参数
		switch (initParam.usage)
		{
		case InitializationParam::REALTIMERENDER:

			windowStartX = (MainMonitor::getWidth() - initParam.width) / 2u;

			windowStartY = (MainMonitor::getHeight() - initParam.height) / 2u;

			windowWidth = initParam.width;

			windowHeight = initParam.height;

			windowStyle = Window::Win32Form::normalWindowStyle;

			windowCallback = Window::Win32Form::realTimeRenderCallback;

			LOGENGINE() << "引擎用途" << "实时渲染";

			break;

		case InitializationParam::VIDEORENDER:

			windowStartX = 100u;

			windowStartY = 100u;

			windowWidth = 100u;

			windowHeight = 100u;

			windowStyle = Window::Win32Form::normalWindowStyle;

			windowCallback = Window::Win32Form::videoRenderCallback;

			LOGENGINE() << "引擎用途" << "视频渲染";

			break;

		case InitializationParam::WALLPAPER:

			initParam.width = MainMonitor::getWidth();

			initParam.height = MainMonitor::getHeight();

			windowStartX = 0u;

			windowStartY = 0u;

			windowWidth = initParam.width;

			windowHeight = initParam.height;

			windowStyle = Window::Win32Form::wallpaperWindowStyle;

			windowCallback = Window::Win32Form::wallpaperCallBack;

			LOGENGINE() << "引擎用途" << "动态壁纸";

			break;

		default:
			break;
		}

		windowToken = makeUnique<Window::Win32Form::InitializeToken>(param.title, windowStartX, windowStartY, windowWidth, windowHeight, windowStyle, windowCallback);

		if (initParam.usage == InitializationParam::WALLPAPER)
		{
			const HWND parentHWND = WallpaperHelper::getWallpaperHWND();

			SetParent(Window::Win32Form::getHandle(), parentHWND);
		}
		else if (initParam.usage == InitializationParam::VIDEORENDER)
		{
			ShowWindow(Window::Win32Form::getHandle(), SW_HIDE);
		}

		const bool useSwapChainBuffer = (initParam.usage != InitializationParam::VIDEORENDER);

		renderEngineToken = makeUnique<RenderEngine::Internal::InitializeToken>(initParam.width, initParam.height, Window::Win32Form::getHandle(), useSwapChainBuffer, initParam.enableImGuiSurface);

		LOGENGINE() << "分辨率" << Graphics::getWidth() << "x" << Graphics::getHeight();

		LOGENGINE() << "横纵比" << Graphics::getAspectRatio();

		LOGENGINE() << "后备缓冲数量" << Graphics::getFrameBufferCount();
	}

	GearImpl::~GearImpl()
	{
		RenderEngine::Internal::waitDestroyable();

#ifdef _DEBUG

		//由于Debug模式下需要输出存活的对象，所以需要手动释放相关资源
		backBufferHeap.reset();

		game.reset();

		renderEngineToken.reset();

		reportLiveObjects();

#endif // _DEBUG

		LOGENGINE() << "资源销毁完毕";
	}

	void GearImpl::initGame(UniquePtr<Game> gamePtr)
	{
		game = std::move(gamePtr);

		RenderEngine::Internal::initializeResources();

		switch (initParam.usage)
		{
		case InitializationParam::REALTIMERENDER:
			return runRealTimeRender();
		case InitializationParam::VIDEORENDER:
			return runVideoRender();
		case InitializationParam::WALLPAPER:
			return runWallpaper();
		default:
			return;
		}
	}

	void GearImpl::runRealTimeRender()
	{
		SetForegroundWindow(Window::Win32Form::getHandle());

		DeltaTimeEstimator deltaTimeEstimator;

		std::chrono::high_resolution_clock::time_point startPoint = std::chrono::high_resolution_clock::now();

		while (Window::Win32Form::pollEvents())
		{
			const std::chrono::high_resolution_clock::time_point endPoint = std::chrono::high_resolution_clock::now();

			const float deltaTime = std::chrono::duration<float>(endPoint - startPoint).count();

			RenderEngine::Internal::setDeltaTime(deltaTimeEstimator.getDeltaTime(deltaTime));

			RenderEngine::Internal::updateTimeElapsed();

			RenderEngine::Internal::setFrameRate(deltaTimeEstimator.getFrameRate());

			startPoint = endPoint;

			//更新帧索引，使用交换链指定的帧缓冲
			RenderEngine::Internal::updateFrameIndex();

			RenderEngine::Internal::waitFrameCPUReusable();

			RenderEngine::Internal::setDefRenderTexture();

			RenderEngine::Internal::beginFrame();

			game->update(Graphics::getDeltaTime());

			game->render();

			const bool needScreenGrab = Input::Keyboard::onKeyDown(screenGrabKey);

			if (needScreenGrab)
			{
				RenderEngine::Internal::saveBackBuffer(backBufferHeap, backBufferFootprint);
			}

			RenderEngine::Internal::endFrame();

			RenderEngine::Internal::present();

			if (needScreenGrab)
			{
				RenderEngine::Internal::waitFrameGPUComplete();
			}

			RenderEngine::Internal::renderedFrameCountInc();

			if (needScreenGrab)
			{
				const uint8_t* const dataPtr = reinterpret_cast<const uint8_t*>(backBufferHeap->map());

				UniquePtr<uint8_t[]> colors = makeUnique<uint8_t[]>(FMT::getByteSize(Graphics::backBufferFormat) * initParam.width * initParam.height);

				for (uint32_t row = 0; row < initParam.height; row++)
				{
					for (uint32_t col = 0; col < initParam.width; col++)
					{
						const uint32_t writePixel = (row * initParam.width + col) * 4u;

						const uint32_t readPixel = static_cast<uint32_t>(backBufferFootprint.Offset) + backBufferFootprint.Footprint.RowPitch * row + 4u * col;

						//RGBA <- BGRA
						colors[writePixel] = dataPtr[readPixel + 2];

						colors[writePixel + 1] = dataPtr[readPixel + 1];

						colors[writePixel + 2] = dataPtr[readPixel];

						colors[writePixel + 3] = 0xFFu;
					}
				}

				backBufferHeap->unmap();

				stbi_write_png("output.png", initParam.width, initParam.height, 4, colors.get(), FMT::getByteSize(Graphics::backBufferFormat) * initParam.width);

				LOGSUCCESS() << "截屏保存到" << COLORIZEPATH("output.png");

				startPoint = std::chrono::high_resolution_clock::now();
			}
		}
	}

	void GearImpl::runVideoRender()
	{
		const AdapterVendor vendor = RenderEngine::getVendor();

		UniquePtr<VideoEncoder::Encoder> encoder;

		const uint32_t frameToEncode = initParam.videoRender.seconds * VideoEncoder::Encoder::frameRate;

		LOGENGINE() << "编码方式" << LogColor::yellow << (initParam.videoRender.hardwareEncode ? "硬件编码" : "软件编码");

		if (initParam.videoRender.hardwareEncode)
		{
			switch (vendor)
			{
			case AdapterVendor::NVIDIA:
				encoder = makeUnique<VideoEncoder::NVIDIAEncoder>(frameToEncode, initParam.videoRender.maxBFrames);
				break;
			case AdapterVendor::AMD:
			case AdapterVendor::INTEL:
			case AdapterVendor::UNKNOWN:
				THROWLOG(LOGERROR() << "目前只为英伟达的GPU做了硬件编码适配");
				break;
			default:
				break;
			}
		}
		else
		{
			encoder = makeUnique<VideoEncoder::SoftwareEncoder>(frameToEncode);
		}

		UniquePtr<D3D12Resource::Texture> renderTexture = makeUnique<D3D12Resource::Texture>(Graphics::getWidth(), Graphics::getHeight(), Graphics::backBufferFormat, 1, 1, true, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, nullptr);

		D3D12_CPU_DESCRIPTOR_HANDLE textureHandle;

		{
			DescriptorHandle descriptorHandle = LocalDescriptorHeap::getRenderTargetHeap()->allocStaticDescriptor(1);

			D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
			rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
			rtvDesc.Format = Graphics::backBufferFormat;
			rtvDesc.Texture2D.MipSlice = 0;
			rtvDesc.Texture2D.PlaneSlice = 0;

			GraphicsDevice::get()->CreateRenderTargetView(renderTexture->getResource(), &rtvDesc, descriptorHandle.getCurrentCPUHandle());

			textureHandle = descriptorHandle.getCurrentCPUHandle();
		}

		RenderEngine::Internal::setDeltaTime(1.f / static_cast<float>(VideoEncoder::Encoder::frameRate));

		RenderEngine::Internal::setFrameRate(VideoEncoder::Encoder::frameRate);

		D3D12Core::FencePtr vpSyncFence = makeUnique<D3D12Core::Fence>();

		bool encoding = true;

		do
		{
			RenderEngine::Internal::setRenderTexture(renderTexture.get(), textureHandle);

			RenderEngine::Internal::beginFrame();

			game->update(Graphics::getDeltaTime());

			game->render();

			RenderEngine::Internal::endFrame();

			RenderEngine::Internal::waitFrameGPUComplete();

			D3D12Core::CommandQueue* const vpCommandQueue = encoder->getVPCommandQueue();

			//编码器管理的视频处理命令队列需要等待渲染引擎管理的图形命令队列完成工作
			vpCommandQueue->waitFor(RenderEngine::getCommandQueue(), vpSyncFence.get());

			encoding = encoder->encode(RenderEngine::getRenderTexture());

			RenderEngine::Internal::updateTimeElapsed();

			RenderEngine::Internal::renderedFrameCountInc();

		} while (encoding);
	}

	void GearImpl::runWallpaper()
	{
		RenderEngine::Internal::setSyncInterval(0);

		const HANDLE timerHandle = CreateWaitableTimerExW(nullptr, nullptr, CREATE_WAITABLE_TIMER_HIGH_RESOLUTION, TIMER_ALL_ACCESS);

		if (!timerHandle)
		{
			THROWLOG(LOGERROR() << "创建帧率限制计时器失败");
		}

		DeltaTimeEstimator deltaTimeEstimator;

		WallpaperHelper::DetectThreadToken detectThreadToken;

		std::chrono::high_resolution_clock::time_point startPoint = std::chrono::high_resolution_clock::now();

		std::chrono::high_resolution_clock::time_point targetEndPoint = startPoint;

		const std::chrono::high_resolution_clock::duration targetFrameDuration = std::chrono::duration_cast<std::chrono::high_resolution_clock::duration>(
			std::chrono::duration<double>(1.0 / static_cast<double>(initParam.wallpaper.targetFrameRate)));

		while (Window::Win32Form::pollEvents())
		{
			while (WallpaperHelper::isDesktopObscured())
			{
				if (!Window::Win32Form::pollEvents(static_cast<DWORD>(WallpaperHelper::obscureCheckInterval / 2ull - 75ull)))
				{
					CloseHandle(timerHandle);

					return;
				}

				startPoint = std::chrono::high_resolution_clock::now();

				targetEndPoint = startPoint;
			}

			const std::chrono::high_resolution_clock::time_point endPoint = std::chrono::high_resolution_clock::now();

			const float deltaTime = std::chrono::duration<float>(endPoint - startPoint).count();

			RenderEngine::Internal::setDeltaTime(deltaTimeEstimator.getDeltaTime(deltaTime));

			RenderEngine::Internal::updateTimeElapsed();

			RenderEngine::Internal::setFrameRate(deltaTimeEstimator.getFrameRate());

			startPoint = endPoint;

			//更新帧索引，使用交换链指定的帧缓冲
			RenderEngine::Internal::updateFrameIndex();

			RenderEngine::Internal::waitFrameCPUReusable();

			RenderEngine::Internal::setDefRenderTexture();

			RenderEngine::Internal::beginFrame();

			game->update(Graphics::getDeltaTime());

			game->render();

			RenderEngine::Internal::endFrame();

			RenderEngine::Internal::present();

			RenderEngine::Internal::renderedFrameCountInc();

			targetEndPoint += targetFrameDuration;

			const std::chrono::high_resolution_clock::time_point currentTimePoint = std::chrono::high_resolution_clock::now();

			if (currentTimePoint < targetEndPoint)
			{
				const long long remainingUs = std::chrono::duration_cast<std::chrono::microseconds>(targetEndPoint - currentTimePoint).count();

				LARGE_INTEGER li;

				li.QuadPart = remainingUs * -10ll;

				SetWaitableTimer(timerHandle, &li, 0, nullptr, nullptr, FALSE);

				WaitForSingleObject(timerHandle, INFINITE);

				while (std::chrono::high_resolution_clock::now() < targetEndPoint)
				{
					if (!Window::Win32Form::pollEvents())
					{
						CloseHandle(timerHandle);

						return;
					}
				}
			}
			else if (currentTimePoint - targetEndPoint > targetFrameDuration)
			{
				targetEndPoint = currentTimePoint;
			}
		}

		CloseHandle(timerHandle);
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

	void initialize(const InitializationParam& param, const int32_t argc, const wchar_t* argv[])
	{
		impl = makeUnique<GearImpl>(param, argc, argv);
	}

	void release()
	{
		impl.reset();
	}

	void initGame(UniquePtr<Game> gamePtr)
	{
		impl->initGame(std::move(gamePtr));
	}

	void failureExit(const std::exception& e)
	{
		//保证错误信息被输出到log.txt
		Logger::Internal::release();

		std::cerr << e.what();

		std::cin.get();

		std::quick_exit(EXIT_FAILURE);
	}
}
