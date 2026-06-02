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

namespace
{
	class GearImpl
	{
	public:

		GearImpl(const GearImpl&) = delete;

		void operator=(const GearImpl&) = delete;

		GearImpl();

		~GearImpl();

		int32_t iniEngine(const Gear::InitializationParam& param, const int32_t argc, const wchar_t* argv[]);

		void iniGame(Gear::Game* const gamePtr);

	private:

		void runRealTimeRender();

		void runVideoRender();

		void runWallpaper();

		void reportLiveObjects() const;

		Gear::Game* game;

		//用于截屏
		Gear::Core::Resource::D3D12Resource::ReadbackHeap* backBufferHeap;

		Gear::InitializationParam::EngineUsage usage;

		union
		{
			Gear::InitializationParam::RealTimeRenderParam realTimeRender;

			Gear::InitializationParam::VideoRenderParam videoRender;
		};

		static constexpr Gear::Input::Keyboard::Key screenGrabKey = Gear::Input::Keyboard::F11;

	}*impl = nullptr;
}

GearImpl::GearImpl() :
	game(nullptr), backBufferHeap(nullptr)
{

}

GearImpl::~GearImpl()
{
	LOGENGINE(L"destroy resources");

	Gear::Core::RenderEngine::Internal::waitForCurrentFrame();

	if (backBufferHeap)
	{
		delete backBufferHeap;
	}

	if (game)
	{
		delete game;
	}

	Gear::Core::RenderEngine::Internal::release();

	Gear::Window::Win32Form::release();

	LOGSUCCESS(L"engine exit");

	Gear::Utils::Logger::Internal::release();

#ifdef _DEBUG

	reportLiveObjects();

#endif // _DEBUG
}

int32_t GearImpl::iniEngine(const Gear::InitializationParam& param, const int32_t argc, const wchar_t* argv[])
{
	Gear::Utils::Logger::Internal::initialize();

	Gear::Utils::File::Internal::setRootFolder(Gear::Utils::File::backslashToSlash(Gear::Utils::File::getParentFolder(argv[0])));

	LOGENGINE(L"root folder", LogColor::brightBlue, Gear::Utils::File::getRootFolder());

	usage = param.usage;

	DirectX::XMUINT2 systemResolution = {};

	Gear::Utils::WallpaperHelper::getSystemResolution(systemResolution.x, systemResolution.y);

	switch (usage)
	{
	case Gear::InitializationParam::EngineUsage::REALTIMERENDER:

		realTimeRender = param.realTimeRender;

		Gear::Window::Win32Form::initialize(param.title, (systemResolution.x - realTimeRender.width) / 2, (systemResolution.y - realTimeRender.height) / 2,
			realTimeRender.width, realTimeRender.height, Gear::Window::Win32Form::normalWindowStyle, Gear::Window::Win32Form::windowCallback);

		Gear::Core::RenderEngine::Internal::initialize(realTimeRender.width, realTimeRender.height, Gear::Window::Win32Form::getHandle(), true, realTimeRender.enableImGuiSurface);

		backBufferHeap = new Gear::Core::Resource::D3D12Resource::ReadbackHeap(Gear::Core::FMT::getByteSize(Gear::Core::Graphics::backBufferFormat) * realTimeRender.width * realTimeRender.height);

		LOGENGINE(L"engine usage real time render");

		break;

	case Gear::InitializationParam::EngineUsage::VIDEORENDER:

		videoRender = param.videoRender;

		Gear::Window::Win32Form::initialize(param.title, 100, 100, 100, 100, Gear::Window::Win32Form::normalWindowStyle, Gear::Window::Win32Form::encodeCallback);

		ShowWindow(Gear::Window::Win32Form::getHandle(), SW_HIDE);

		Gear::Core::RenderEngine::Internal::initialize(videoRender.width, videoRender.height, Gear::Window::Win32Form::getHandle(), false, false);

		LOGENGINE(L"engine usage video render");

		break;

	case Gear::InitializationParam::EngineUsage::WALLPAPER:

		Gear::Window::Win32Form::initialize(param.title, 0, 0, systemResolution.x, systemResolution.y, Gear::Window::Win32Form::wallpaperWindowStyle, Gear::Window::Win32Form::wallpaperCallBack);

		{
			const HWND parentHWND = Gear::Utils::WallpaperHelper::getWallpaperHWND();

			SetParent(Gear::Window::Win32Form::getHandle(), parentHWND);
		}

		Gear::Core::RenderEngine::Internal::initialize(systemResolution.x, systemResolution.y, Gear::Window::Win32Form::getHandle(), true, false);

		LOGENGINE(L"engine usage wallpaper");

		break;

	default:
		break;
	}

	LOGENGINE(L"resolution", Gear::Core::Graphics::getWidth(), L"x", Gear::Core::Graphics::getHeight());

	LOGENGINE(L"aspect ratio", Gear::Core::Graphics::getAspectRatio());

	LOGENGINE(L"back buffer count", Gear::Core::Graphics::getFrameBufferCount());

	return 0;
}

void GearImpl::iniGame(Gear::Game* const gamePtr)
{
	game = gamePtr;

	Gear::Core::RenderEngine::Internal::initializeResources();

	switch (usage)
	{
	case Gear::InitializationParam::EngineUsage::REALTIMERENDER:
		runRealTimeRender();
		break;

	case Gear::InitializationParam::EngineUsage::VIDEORENDER:
		runVideoRender();
		break;

	case Gear::InitializationParam::EngineUsage::WALLPAPER:
		runWallpaper();
		break;

	default:
		break;
	}
}

void GearImpl::runRealTimeRender()
{
	Gear::Utils::DeltaTimeEstimator dtEstimator;

	while (Gear::Window::Win32Form::pollEvents())
	{
		const std::chrono::high_resolution_clock::time_point startPoint = std::chrono::high_resolution_clock::now();

		Gear::Core::RenderEngine::Internal::setDefRenderTexture();

		Gear::Core::RenderEngine::Internal::begin();

		game->update(Gear::Core::Graphics::getDeltaTime());

		game->render();

		const bool needScreenGrab = Gear::Input::Keyboard::onKeyDown(screenGrabKey);

		if (needScreenGrab)
		{
			Gear::Core::RenderEngine::Internal::saveBackBuffer(backBufferHeap);
		}

		Gear::Core::RenderEngine::Internal::end();

		Gear::Core::RenderEngine::Internal::present();

		if (needScreenGrab)
		{
			Gear::Core::RenderEngine::Internal::waitForCurrentFrame();
		}

		Gear::Core::RenderEngine::Internal::waitForNextFrame();

		const std::chrono::high_resolution_clock::time_point endPoint = std::chrono::high_resolution_clock::now();

		const float deltaTime = std::chrono::duration<float>(endPoint - startPoint).count();

		const float lerpDeltaTime = dtEstimator.getDeltaTime(deltaTime);

		Gear::Core::RenderEngine::Internal::setDeltaTime(lerpDeltaTime);

		Gear::Core::RenderEngine::Internal::updateTimeElapsed();

		if (needScreenGrab)
		{
			const uint8_t* const dataPtr = reinterpret_cast<uint8_t*>(backBufferHeap->map(CD3DX12_RANGE(0ull,
				Gear::Core::FMT::getByteSize(Gear::Core::Graphics::backBufferFormat) * realTimeRender.width * realTimeRender.height)));

			uint8_t* const colors = new uint8_t[Gear::Core::FMT::getByteSize(Gear::Core::Graphics::backBufferFormat) * realTimeRender.width * realTimeRender.height];

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

			stbi_write_png("output.png", realTimeRender.width, realTimeRender.height, 4, colors, Gear::Core::FMT::getByteSize(Gear::Core::Graphics::backBufferFormat) * realTimeRender.width);

			delete[] colors;

			LOGSUCCESS(L"screenshot saved to output.png");
		}
	}
}

void GearImpl::runVideoRender()
{
	const Gear::Core::GPUVendor vendor = Gear::Core::RenderEngine::getVendor();

	Gear::Core::VideoEncoder::Encoder* encoder = nullptr;

	const uint32_t frameToEncode = videoRender.second * Gear::Core::VideoEncoder::Encoder::frameRate;

	switch (vendor)
	{
	case Gear::Core::GPUVendor::NVIDIA:
		encoder = new Gear::Core::VideoEncoder::NvidiaEncoder(frameToEncode);
		break;
	case Gear::Core::GPUVendor::AMD:
	case Gear::Core::GPUVendor::INTEL:
	case Gear::Core::GPUVendor::UNKNOWN:
		break;
	default:
		break;
	}

	if (vendor == Gear::Core::GPUVendor::NVIDIA)
	{
		const uint32_t numTextures = Gear::Core::VideoEncoder::NvidiaEncoder::lookaheadDepth + 1;

		Gear::Core::Resource::D3D12Resource::Texture* renderTextures[numTextures] = {};

		D3D12_CPU_DESCRIPTOR_HANDLE textureHandles[numTextures] = {};

		{
			Gear::Core::D3D12Core::DescriptorHandle descriptorHandle = Gear::Core::LocalDescriptorHeap::getRenderTargetHeap()->allocStaticDescriptor(numTextures);

			D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
			rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
			rtvDesc.Format = Gear::Core::Graphics::backBufferFormat;
			rtvDesc.Texture2D.MipSlice = 0;
			rtvDesc.Texture2D.PlaneSlice = 0;

			for (uint32_t i = 0; i < numTextures; i++)
			{
				const D3D12_CLEAR_VALUE clearValue = { Gear::Core::Graphics::backBufferFormat ,{0.f,0.f,0.f,1.f} };

				renderTextures[i] = new Gear::Core::Resource::D3D12Resource::Texture(Gear::Core::Graphics::getWidth(), Gear::Core::Graphics::getHeight(), Gear::Core::Graphics::backBufferFormat, 1, 1, true, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, &clearValue);

				Gear::Core::GraphicsDevice::get()->CreateRenderTargetView(renderTextures[i]->getResource(), &rtvDesc, descriptorHandle.getCurrentCPUHandle());

				textureHandles[i] = descriptorHandle.getCurrentCPUHandle();

				descriptorHandle.move();
			}
		}

		uint32_t index = 0;

		Gear::Core::RenderEngine::Internal::setDeltaTime(1.f / static_cast<float>(Gear::Core::VideoEncoder::Encoder::frameRate));

		while (true)
		{
			Gear::Core::RenderEngine::Internal::setRenderTexture(renderTextures[index], textureHandles[index]);

			Gear::Core::RenderEngine::Internal::begin();

			game->update(Gear::Core::Graphics::getDeltaTime());

			game->render();

			Gear::Core::RenderEngine::Internal::end();

			Gear::Core::RenderEngine::Internal::waitForCurrentFrame();

			Gear::Core::RenderEngine::Internal::updateTimeElapsed();

			if (!encoder->encode(Gear::Core::RenderEngine::getRenderTexture()))
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
	Gear::Utils::DeltaTimeEstimator dtEstimator;

	while (Gear::Window::Win32Form::pollEvents())
	{
		const std::chrono::high_resolution_clock::time_point startPoint = std::chrono::high_resolution_clock::now();

		Gear::Core::RenderEngine::Internal::setDefRenderTexture();

		Gear::Core::RenderEngine::Internal::begin();

		game->update(Gear::Core::Graphics::getDeltaTime());

		game->render();

		Gear::Core::RenderEngine::Internal::end();

		Gear::Core::RenderEngine::Internal::present();

		Gear::Core::RenderEngine::Internal::waitForNextFrame();

		const std::chrono::high_resolution_clock::time_point endPoint = std::chrono::high_resolution_clock::now();

		const float deltaTime = std::chrono::duration<float>(endPoint - startPoint).count();

		const float lerpDeltaTime = dtEstimator.getDeltaTime(deltaTime);

		Gear::Core::RenderEngine::Internal::setDeltaTime(lerpDeltaTime);

		Gear::Core::RenderEngine::Internal::updateTimeElapsed();
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

int32_t Gear::iniEngine(const InitializationParam& param, const int32_t argc, const wchar_t* argv[])
{
	return impl->iniEngine(param, argc, argv);
}

void Gear::iniGame(Game* const gamePtr)
{
	impl->iniGame(gamePtr);
}

void Gear::initialize()
{
	impl = new GearImpl();
}

void Gear::release()
{
	if (impl)
	{
		delete impl;
	}
}

void Gear::failureExit()
{
	Gear::Utils::Logger::Internal::release();

	std::quick_exit(EXIT_FAILURE);
}
