#include<Gear/Core/RenderEngine.h>

#include<Gear/Core/Internal/RenderEngineInternal.h>

#include<Gear/Core/Device.h>

#include<Gear/Core/Internal/DeviceInternal.h>

#include<Gear/Core/Graphics.h>

#include<Gear/Core/Internal/GraphicsInternal.h>

#include<Gear/Core/MainCamera.h>

#include<Gear/Core/GraphicsDevice.h>

#include<Gear/Core/Internal/FMTInternal.h>

#include<Gear/Utils/Random.h>

#include<Gear/Utils/File.h>

#include<Gear/Utils/MainMonitor.h>

#include<Gear/Core/D3D12Core/CommandQueue.h>

#include<Gear/Core/D3D12Core/CommonShaderLayout.h>

#include<Gear/Core/Internal/RenderThreadLocalInternal.h>

#include<Gear/Core/Internal/RenderThreadGlobalInternal.h>

#include<Gear/Core/Internal/DynamicCBufferManagerInternal.h>

#include<Gear/Effect/Internal/BackBufferBlitEffectInternal.h>

#include<Gear/Effect/Internal/HDRClampEffectInternal.h>

#include<Gear/Effect/Internal/LatLongMapToCubeMapEffectInternal.h>

#include<Gear/Effect/Internal/ToneMapEffectInternal.h>

#include<Gear/Effect/Internal/GammaCorrectEffectInternal.h>

#include<ImGUI/imgui.h>

#include<ImGUI/imgui_impl_win32.h>

#include<ImGUI/imgui_impl_dx12.h>

#include<dxgi1_6.h>

namespace Gear::Core::RenderEngine
{
	struct ImGuiToken
	{
		ImGuiToken(const HWND hWnd, ImFont** mediumFont, ImFont** largeFont)
		{
			IMGUI_CHECKVERSION();
			ImGui::CreateContext();
			ImGuiIO& io = ImGui::GetIO();
			(void)io;

			ImGui::StyleColorsDark();

			ImGui::GetStyle().ScaleAllSizes(Utils::MainMonitor::getScale());

			const D3D12Core::DescriptorHandle handle = GlobalDescriptorHeap::getResourceHeap()->allocStaticDescriptor(1);

			ImGui_ImplWin32_Init(hWnd);
			ImGui_ImplDX12_Init(GraphicsDevice::get(), Graphics::getFrameBufferCount(), Graphics::backBufferFormat,
				GlobalDescriptorHeap::getResourceHeap()->get(), handle.getCurrentCPUHandle(), handle.getCurrentGPUHandle());

			//显示输入法的待选框
			ImGui::GetMainViewport()->PlatformHandleRaw = (void*)hWnd;

			ImFontGlyphRangesBuilder builder;

			//加载常用汉字，GetGlyphRangesChineseSimplifiedCommon提供的汉字完全不够
			std::vector<uint8_t> chineseCharacters = Utils::File::readAllBinary(Utils::File::EngineAssetsPath(L"7000+symbols.txt"));

			chineseCharacters.push_back('\0');

			builder.AddText(reinterpret_cast<const char*>(chineseCharacters.data()));

			//加载常用字符
			builder.AddRanges(io.Fonts->GetGlyphRangesDefault());

			ImVector<ImWchar> ranges;

			builder.BuildRanges(&ranges);

			//加载微软雅黑字体
			*mediumFont = io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/msyh.ttc", 18.f * Utils::MainMonitor::getScale(), nullptr, ranges.Data);

			*largeFont = io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/msyh.ttc", 22.f * Utils::MainMonitor::getScale(), nullptr, ranges.Data);

			io.FontDefault = *mediumFont;

			io.Fonts->GetTexDataAsRGBA32(nullptr, nullptr, nullptr);
		}

		~ImGuiToken()
		{
			ImGui_ImplDX12_Shutdown();
			ImGui_ImplWin32_Shutdown();
			ImGui::DestroyContext();
		}
	};

	struct RenderResourceToken
	{
		RenderResourceToken(ResourceManager* const resManager) :
			latLongMapToCubeMapEffect(resManager)
		{

		}

		Effect::BackBufferBlitEffect::Internal::InitializeToken backBufferBlitEffect;

		Effect::HDRClampEffect::Internal::InitializeToken hdrClampEffect;

		Effect::LatLongMapToCubeMapEffect::Internal::InitializeToken latLongMapToCubeMapEffect;

		Effect::ToneMapEffect::Internal::InitializeToken toneMapEffect;

		Effect::GammaCorrectEffect::Internal::InitializeToken gammaCorrectEffect;
	};

	class RenderEngineImpl
	{
	public:

		RenderEngineImpl() = delete;

		RenderEngineImpl(const RenderEngineImpl&) = delete;

		void operator=(const RenderEngineImpl&) = delete;

		RenderEngineImpl(const uint32_t width, const uint32_t height, const HWND hWnd, const bool useSwapChainBuffer, const bool initializeImGuiSurface);

		~RenderEngineImpl();

		void submitCommandList(D3D12Core::CommandList* const commandList);

		AdapterVendor getVendor() const;

		D3D12Resource::Texture* getRenderTexture() const;

		D3D12_CPU_DESCRIPTOR_HANDLE getBackBufferHandle() const;

		D3D12Core::CommandQueue* getCommandQueue() const;

		const Resource::ImmutableCBuffer* getEngineGlobalCBuffer() const;

		void updateFrameIndex();

		void waitDestroyable();

		void waitFrameCPUReusable();

		void waitFrameGPUComplete();

		void beginFrame();

		void endFrame();

		void processCommandLists();

		void present() const;

		void setSyncInterval(int32_t interval);

		void setDefRenderTexture();

		void setRenderTexture(D3D12Resource::Texture* const texture, const D3D12_CPU_DESCRIPTOR_HANDLE handle);

		void initializeResources();

		void saveBackBuffer(D3D12Resource::ReadbackHeapPtr& readbackHeap, D3D12_PLACED_SUBRESOURCE_FOOTPRINT& backBufferFootprint);

		bool getDisplayImGuiSurface() const;

		void toggleImGuiSurface();

		void toggleEngineImGuiSurface();

		ImFont* getMediumFont() const;

		ImFont* getLargeFont() const;

	private:

		ComPtr<IDXGIAdapter4> getBestAdapterAndVendor(IDXGIFactory7* const factory);

		void updateDynamicCBuffers() const;

		void beginImGuiFrame() const;

		void drawImGuiFrame();

		UniquePtr<Device::Internal::InitializeToken> deviceToken;

		UniquePtr<D3D12Core::GraphicsCommandList> prepareGraphicsCommandList;

		D3D12Core::CommandQueuePtr commandQueue;

		D3D12Core::GraphicsCommandList* lastUsableGraphicsCommandList;

		UniquePtr<RenderThreadLocal::Internal::InitializeToken> renderThreadLocalToken;

		UniquePtr<RenderThreadGlobal::Internal::InitializeToken> renderThreadGlobalToken;

		UniquePtr<Resource::DynamicCBuffer> engineGlobalCBuffer;

		UniquePtr<ResourceManager> resManager;

		UniquePtr<RenderResourceToken> renderResourceToken;

		ComPtr<IDXGISwapChain4> swapChain;

		UniquePtr<D3D12_CPU_DESCRIPTOR_HANDLE[]> backBufferHandles;

		UniquePtr<D3D12Resource::TexturePtr[]> backBufferTextures;

		const bool initializeImGuiSurface;

		bool displayImGuiSurface;

		bool displayEngineImGuiSurface;

		AdapterVendor vendor;

		UniquePtr<ImGuiToken> imGuiToken;

		ImFont* mediumFont;

		ImFont* largeFont;

		//引用
		D3D12Resource::Texture* renderTexture;

		//引用
		D3D12_CPU_DESCRIPTOR_HANDLE backBufferHandle;

		int32_t syncInterval;

		D3D12Core::CommonShaderLayout::PerframeResource perframeResource;

	};

	RenderEngineImpl::RenderEngineImpl(const uint32_t width, const uint32_t height, const HWND hWnd, const bool useSwapChainBuffer, const bool initializeImGuiSurface) :
		vendor(AdapterVendor::UNKNOWN),
		lastUsableGraphicsCommandList(nullptr),
		initializeImGuiSurface(initializeImGuiSurface),
		displayImGuiSurface(false),
		displayEngineImGuiSurface(true),
		mediumFont(nullptr),
		largeFont(nullptr),
		renderTexture(nullptr),
		backBufferHandle{},
		syncInterval(1),
		perframeResource{}
	{
		//初始化一些渲染需要的信息，如width、height、frameIndex等
		Graphics::Internal::initialize(useSwapChainBuffer ? 3 : 1, width, height);

		ComPtr<IDXGIFactory7> factory;

#ifdef _DEBUG
		LOGENGINE() << COLORIZE("开启", LogColor::brightGreen) << COLORIZE("调试层", LogColor::brightMagenta);

		ComPtr<ID3D12Debug> debugController;

		D3D12GetDebugInterface(IID_PPV_ARGS(&debugController));

		debugController->EnableDebugLayer();

		CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&factory));
#else
		LOGENGINE() << COLORIZE("关闭", LogColor::brightRed) << COLORIZE("调试层", LogColor::brightMagenta);

		CreateDXGIFactory2(0, IID_PPV_ARGS(&factory));
#endif // _DEBUG

		//获取适配器
		ComPtr<IDXGIAdapter4> adapter = getBestAdapterAndVendor(factory.Get());

		//传入适配器，初始化图形设备(ID3D12Device)
		deviceToken = makeUnique<Device::Internal::InitializeToken>(adapter.Get());

		//检查并输出一些特性的支持情况
		//不支持Shader Model 6.6或有类型UAV读取会报错
		Device::checkFeatureSupport();

		//初始化格式表用于查询
		FMT::Internal::initialize();

		//创建准备命令列表
		prepareGraphicsCommandList = makeUnique<D3D12Core::GraphicsCommandList>();

		//创建命令队列
		commandQueue = makeUnique<D3D12Core::CommandQueue>(prepareGraphicsCommandList.get());

		commandQueue->setName(L"Graphics Command Queue");

		//初始化线程局部资源
		renderThreadLocalToken = makeUnique<RenderThreadLocal::Internal::InitializeToken>();

		//初始化线程全局资源
		renderThreadGlobalToken = makeUnique<RenderThreadGlobal::Internal::InitializeToken>();

		//引擎需要使用一个动态常量缓冲为每一帧的渲染提供有用的信息
		engineGlobalCBuffer = ResourceManager::createDynamicCBuffer(sizeof(perframeResource));

		//把准备命令列表推入容器中，因为资源的初始化可能需要动态常量缓冲
		//而动态常量缓冲更新的指令记录是由prepareCommandList负责的
		commandQueue->begin();

		resManager = makeUnique<ResourceManager>();

		{
			GraphicsContext* const context = resManager->getGraphicsContext();

			context->begin();

			renderResourceToken = makeUnique<RenderResourceToken>(resManager.get());

			submitCommandList(resManager->getCommandList());
		}

		//创建交换链
		{
			DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
			swapChainDesc.BufferCount = Utils::Math::max(Graphics::getFrameBufferCount(), 2u);
			swapChainDesc.Width = Graphics::getWidth();
			swapChainDesc.Height = Graphics::getHeight();
			swapChainDesc.Format = Graphics::backBufferFormat;
			swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
			swapChainDesc.SampleDesc.Count = 1;
			swapChainDesc.SampleDesc.Quality = 0;

			ComPtr<IDXGISwapChain1> swapChain1;

			factory->CreateSwapChainForHwnd(commandQueue->get(), hWnd, &swapChainDesc, nullptr, nullptr, &swapChain1);

			factory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER);

			swapChain1.As(&swapChain);
		}

		//如果需要使用交换链的后备缓冲
		//那么需要取出纹理用于状态追踪并为其纹理创建RTV
		if (useSwapChainBuffer)
		{
			D3D12Core::DescriptorHandle descriptorHandle = LocalDescriptorHeap::getRenderTargetHeap()->allocStaticDescriptor(Graphics::getFrameBufferCount());

			backBufferHandles = makeUnique<D3D12_CPU_DESCRIPTOR_HANDLE[]>(Graphics::getFrameBufferCount());

			backBufferTextures = makeUnique<D3D12Resource::TexturePtr[]>(Graphics::getFrameBufferCount());

			for (uint32_t i = 0; i < Graphics::getFrameBufferCount(); i++)
			{
				ComPtr<ID3D12Resource> texture;

				swapChain->GetBuffer(i, IID_PPV_ARGS(&texture));

				const std::wstring backBufferName = L"Back Buffer (" + std::to_wstring(i) + L")";

				texture->SetName(backBufferName.c_str());

				GraphicsDevice::get()->CreateRenderTargetView(texture.Get(), nullptr, descriptorHandle.getCurrentCPUHandle());

				backBufferHandles[i] = descriptorHandle.getCurrentCPUHandle();

				descriptorHandle.move();

				//后备缓冲的初态为D3D12_RESOURCE_STATE_PRESENT
				backBufferTextures[i] = makeUnique<D3D12Resource::Texture>(texture, true, D3D12_RESOURCE_STATE_PRESENT);
			}
		}

		//如果有需要，那么初始化ImGUI
		if (initializeImGuiSurface)
		{
			LOGENGINE() << COLORIZE("开启", LogColor::brightGreen) << COLORIZE("ImGui", LogColor::brightMagenta);

			imGuiToken = makeUnique<ImGuiToken>(hWnd, &mediumFont, &largeFont);
		}
		else
		{
			LOGENGINE() << COLORIZE("关闭", LogColor::brightRed) << COLORIZE("ImGui", LogColor::brightMagenta);
		}

		//设置默认的2D投影矩阵
		MainCamera::setProj(DirectX::XMMatrixOrthographicOffCenterLH(0.f, static_cast<float>(Graphics::getWidth()), 0, static_cast<float>(Graphics::getHeight()), -1.f, 1.f));

		//设置默认的视图矩阵
		MainCamera::setView(DirectX::XMMatrixIdentity());
	}

	RenderEngineImpl::~RenderEngineImpl()
	{
	}

	void RenderEngineImpl::submitCommandList(D3D12Core::CommandList* const commandList)
	{
		commandQueue->submitCommandList(commandList);
	}

	AdapterVendor RenderEngineImpl::getVendor() const
	{
		return vendor;
	}

	D3D12Resource::Texture* RenderEngineImpl::getRenderTexture() const
	{
		return renderTexture;
	}

	D3D12_CPU_DESCRIPTOR_HANDLE RenderEngineImpl::getBackBufferHandle() const
	{
		return backBufferHandle;
	}

	D3D12Core::CommandQueue* RenderEngineImpl::getCommandQueue() const
	{
		return commandQueue.get();
	}

	const Resource::ImmutableCBuffer* RenderEngineImpl::getEngineGlobalCBuffer() const
	{
		return engineGlobalCBuffer.get();
	}

	void RenderEngineImpl::updateFrameIndex()
	{
		Graphics::Internal::setFrameIndex(swapChain->GetCurrentBackBufferIndex());
	}

	void RenderEngineImpl::waitDestroyable()
	{
		commandQueue->waitDestroyable();
	}

	void RenderEngineImpl::waitFrameGPUComplete()
	{
		commandQueue->waitFrameGPUComplete();
	}

	void RenderEngineImpl::waitFrameCPUReusable()
	{
		commandQueue->waitFrameCPUReusable();
	}

	void RenderEngineImpl::beginFrame()
	{
		beginImGuiFrame();

		commandQueue->begin();

		lastUsableGraphicsCommandList = nullptr;

		//先获取可用的位置，供GraphicsContext在这一帧使用
		engineGlobalCBuffer->acquireDataPtr();

		//把后备缓冲转变到STATE_RENDER_TARGET，并暂存资源屏障
		prepareGraphicsCommandList->trackAndSetResourceState(getRenderTexture(), D3D12Resource::D3D12_TRANSITION_ALL_MIPLEVELS, D3D12_RESOURCE_STATE_RENDER_TARGET);

		prepareGraphicsCommandList->flushTransitionResources();
	}

	void RenderEngineImpl::endFrame()
	{
		//到这里我们已经知道了哪些动态常量缓冲需要更新
		updateDynamicCBuffers();

		//一些比较基础的信息的设置
		{
			perframeResource.deltaTime = Graphics::getDeltaTime();

			perframeResource.timeElapsed = Graphics::getTimeElapsed();

			perframeResource.uintSeed = Utils::Random::genUint();

			perframeResource.floatSeed = Utils::Random::genFloat();
		}

		//主相机相关信息的设置
		{
			perframeResource.prevViewProj = perframeResource.viewProj;

			perframeResource.proj = DirectX::XMMatrixTranspose(MainCamera::getProj());

			perframeResource.view = DirectX::XMMatrixTranspose(MainCamera::getView());

			perframeResource.viewProj = DirectX::XMMatrixTranspose(MainCamera::getView() * MainCamera::getProj());

			//逆的转置的转置等于没有转置
			perframeResource.normalMatrix = DirectX::XMMatrixInverse(nullptr, MainCamera::getView());

			DirectX::XMStoreFloat4(&perframeResource.cameraPos, MainCamera::getEyePos());

			perframeResource.cameraPos.w = 1.0f;

			//归一化保险一点
			DirectX::XMStoreFloat4(&perframeResource.cameraRight, DirectX::XMVector3Normalize(perframeResource.view.r[0]));

			DirectX::XMStoreFloat4(&perframeResource.cameraUp, DirectX::XMVector3Normalize(perframeResource.view.r[1]));

			DirectX::XMStoreFloat4(&perframeResource.cameraForward, DirectX::XMVector3Normalize(perframeResource.view.r[2]));

			//方向w要置零
			perframeResource.cameraRight.w = 0.0f;

			perframeResource.cameraUp.w = 0.0f;

			perframeResource.cameraForward.w = 0.0f;

			perframeResource.fovAngleY = MainCamera::getFovAngleY();

			perframeResource.zNear = MainCamera::getZNear();

			perframeResource.zFar = MainCamera::getZFar();
		}
		//关于为什么要转置我找到了一篇有关的文章
		//https://www.douduck08.com/zh-tw/why-dx11-need-matrix-transpose-before-cbuffer-mapping/
		//这里简要说一下，其实和矩阵如何被解释有关，矩阵实际上是以一维数组的形式被存储的
		//DirectXMath默认其为Row Major，而HLSL默认其为Column Major
		//在DirectXMath中我们一般使用DirectX::XMVector4Transform，它背后的数学运算是 vec*matrix
		//如果数据原封不动上传到显存上，那么这个矩阵会被HLSL用另一种方式来解释，我们因此需要的数学运算是 matrix*vec，即mul(matrix,vec)
		//然而，mul(vec,matrix)是有一些性能优势的，为了利用这个性能优势，矩阵在上传前要被转置

		//屏幕相关信息的设置
		{
			perframeResource.aspectRatio = Graphics::getAspectRatio();

			perframeResource.screenSize = DirectX::XMFLOAT2(
				static_cast<float>(Graphics::getWidth()),
				static_cast<float>(Graphics::getHeight()));

			perframeResource.screenTexelSize = DirectX::XMFLOAT2(
				1.f / perframeResource.screenSize.x,
				1.f / perframeResource.screenSize.y);
		}

		engineGlobalCBuffer->updateData(&perframeResource);

		//使用收尾命令列表绘制ImGui界面
		drawImGuiFrame();

		{
			D3D12Core::CommandList* const lastUsableCommandList = commandQueue->getLastUsableCommandList();

			//使用最后一个可用的直接类型的命令列表把后备缓冲转变到STATE_PRESENT
			lastUsableCommandList->trackAndSetResourceState(getRenderTexture(), D3D12Resource::D3D12_TRANSITION_ALL_MIPLEVELS, D3D12_RESOURCE_STATE_PRESENT);

			lastUsableCommandList->flushResourceBarriers();
		}

		processCommandLists();
	}

	void RenderEngineImpl::processCommandLists()
	{
		commandQueue->processCommandLists();
	}

	void RenderEngineImpl::present() const
	{
		swapChain->Present(static_cast<uint32_t>(syncInterval), 0);
	}

	void RenderEngineImpl::setSyncInterval(int32_t interval)
	{
		syncInterval = Utils::Math::clamp(interval, 0, 4);
	}

	void RenderEngineImpl::setDefRenderTexture()
	{
		setRenderTexture(backBufferTextures[Graphics::getFrameIndex()].get(), backBufferHandles[Graphics::getFrameIndex()]);
	}

	void RenderEngineImpl::setRenderTexture(D3D12Resource::Texture* const texture, const D3D12_CPU_DESCRIPTOR_HANDLE handle)
	{
		//接管renderTexture的状态转变
		renderTexture = texture;

		//获取CPU描述符句柄供GraphicsContext在这一帧使用
		backBufferHandle = handle;
	}

	void RenderEngineImpl::initializeResources()
	{
		//如果有需要，那么开启ImGui
		toggleImGuiSurface();

		//更新动态常量缓冲，因为资源创建可能会需要动态常量缓冲
		updateDynamicCBuffers();

		processCommandLists();

		//等待准备工作完成
		waitFrameGPUComplete();

		RenderThreadLocal::Internal::flushCopiedResources();

		//清理静态资源管理器创建的临时资源
		resManager->cleanTransientResources();
	}

	void RenderEngineImpl::saveBackBuffer(D3D12Resource::ReadbackHeapPtr& readbackHeap, D3D12_PLACED_SUBRESOURCE_FOOTPRINT& backBufferFootprint)
	{
		const D3D12_RESOURCE_DESC backBufferDesc = getRenderTexture()->getResource()->GetDesc();

		uint64_t backBufferByteSize = 0ull;

		GraphicsDevice::get()->GetCopyableFootprints(&backBufferDesc, 0, 1, 0, &backBufferFootprint, nullptr, nullptr, &backBufferByteSize);

		if (readbackHeap == nullptr || readbackHeap->getSize() != backBufferByteSize)
		{
			readbackHeap = makeUnique<D3D12Resource::ReadbackHeap>(backBufferByteSize);
		}

		const CD3DX12_TEXTURE_COPY_LOCATION copyDest(readbackHeap->getResource(), backBufferFootprint);

		const CD3DX12_TEXTURE_COPY_LOCATION copySrc(getRenderTexture()->getResource(), 0);

		if (!lastUsableGraphicsCommandList)
		{
			lastUsableGraphicsCommandList = static_cast<D3D12Core::GraphicsCommandList*>(commandQueue->getLastUsableCommandList());
		}

		lastUsableGraphicsCommandList->trackAndSetResourceState(getRenderTexture(), D3D12Resource::D3D12_TRANSITION_ALL_MIPLEVELS, D3D12_RESOURCE_STATE_COPY_SOURCE);

		lastUsableGraphicsCommandList->flushResourceBarriers();

		lastUsableGraphicsCommandList->get()->CopyTextureRegion(&copyDest, 0, 0, 0, &copySrc, nullptr);
	}

	bool RenderEngineImpl::getDisplayImGuiSurface() const
	{
		return displayImGuiSurface;
	}

	void RenderEngineImpl::toggleImGuiSurface()
	{
		if (initializeImGuiSurface)
		{
			displayImGuiSurface = !displayImGuiSurface;
		}
	}

	void RenderEngineImpl::toggleEngineImGuiSurface()
	{
		displayEngineImGuiSurface = !displayEngineImGuiSurface;
	}

	ImFont* RenderEngineImpl::getMediumFont() const
	{
		return mediumFont;
	}

	ImFont* RenderEngineImpl::getLargeFont() const
	{
		return largeFont;
	}

	ComPtr<IDXGIAdapter4> RenderEngineImpl::getBestAdapterAndVendor(IDXGIFactory7* const factory)
	{
		ComPtr<IDXGIAdapter4> adapter;

		for (uint32_t adapterIndex = 0;
			SUCCEEDED(factory->EnumAdapterByGpuPreference(adapterIndex, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter)));
			adapterIndex++)
		{
			DXGI_ADAPTER_DESC3 desc = {};

			adapter->GetDesc3(&desc);

			if (desc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE)
			{
				continue;
			}

			if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
			{
				const uint32_t vendorID = desc.VendorId;

				std::string vendorName;

				if (vendorID == 0x10DE)
				{
					vendor = AdapterVendor::NVIDIA;

					vendorName = "NVIDIA";
				}
				else if (vendorID == 0x1002 || vendorID == 0x1022)
				{
					vendor = AdapterVendor::AMD;

					vendorName = "AMD";
				}
				else if (vendorID == 0x163C || vendorID == 0x8086 || vendorID == 0x8087)
				{
					vendor = AdapterVendor::INTEL;

					vendorName = "INTEL";
				}
				else
				{
					vendor = AdapterVendor::UNKNOWN;

					vendorName = "UNKNOWN";
				}

				LOGENGINE() << "以下是适配器的相关信息";

				LOGENGINE() << "适配器名称" << COLORIZE(desc.Description, LogColor::brightMagenta);

				LOGENGINE() << "适配器生产商ID" << LogIntegerMode::HEX << vendorID;

				LOGENGINE() << "适配器生产商" << COLORIZE(vendorName, LogColor::brightMagenta);

				LOGENGINE() << "适配器专有视频内存" << static_cast<float>(desc.DedicatedVideoMemory) / 1024.f / 1024.f / 1024.f << "GB";

				break;
			}
		}

		return adapter;
	}

	void RenderEngineImpl::updateDynamicCBuffers() const
	{
		DynamicCBufferManager::Internal::recordCommands(prepareGraphicsCommandList.get());
	}

	void RenderEngineImpl::beginImGuiFrame() const
	{
		if (displayImGuiSurface)
		{
			ImGui_ImplDX12_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();
		}
	}

	void RenderEngineImpl::drawImGuiFrame()
	{
		if (displayImGuiSurface)
		{
			if (displayEngineImGuiSurface)
			{
				ImGui::Begin("Frame Profile");
				ImGui::Text("TimeElapsed %.2f", Graphics::getTimeElapsed());
				ImGui::Text("FrameTime %.8f", Graphics::getDeltaTime() * 1000.f);
				ImGui::Text("FrameRate %.1f", Graphics::getFrameRate());
				ImGui::SliderInt("Sync Interval", &syncInterval, 0, 4);
				ImGui::End();

				Graphics::Internal::imGuiCall();
			}

			if (!lastUsableGraphicsCommandList)
			{
				lastUsableGraphicsCommandList = static_cast<D3D12Core::GraphicsCommandList*>(commandQueue->getLastUsableCommandList());
			}

			lastUsableGraphicsCommandList->trackAndSetResourceState(getRenderTexture(), D3D12Resource::D3D12_TRANSITION_ALL_MIPLEVELS, D3D12_RESOURCE_STATE_RENDER_TARGET);

			lastUsableGraphicsCommandList->flushResourceBarriers();

			lastUsableGraphicsCommandList->setDescriptorHeap(GlobalDescriptorHeap::getResourceHeap()->get(), GlobalDescriptorHeap::getSamplerHeap()->get());

			ImGui::Render();

			lastUsableGraphicsCommandList->setDefRenderTarget();

			ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), lastUsableGraphicsCommandList->get());
		}
	}

	UniquePtr<RenderEngineImpl> impl;

	namespace Internal
	{
		void initialize(const uint32_t width, const uint32_t height, const HWND hWnd, const bool useSwapChainBuffer, const bool initializeImGuiSurface)
		{
			impl = makeUnique<RenderEngineImpl>(width, height, hWnd, useSwapChainBuffer, initializeImGuiSurface);
		}

		void release()
		{
			impl.reset();
		}

		void updateFrameIndex()
		{
			impl->updateFrameIndex();
		}

		void waitDestroyable()
		{
			impl->waitDestroyable();
		}

		void waitFrameGPUComplete()
		{
			impl->waitFrameGPUComplete();
		}

		void waitFrameCPUReusable()
		{
			impl->waitFrameCPUReusable();
		}

		void beginFrame()
		{
			impl->beginFrame();
		}

		void endFrame()
		{
			impl->endFrame();
		}

		void processCommandLists()
		{
			impl->processCommandLists();
		}

		void present()
		{
			impl->present();
		}

		void setSyncInterval(const int32_t syncInterval)
		{
			impl->setSyncInterval(syncInterval);
		}

		void saveBackBuffer(D3D12Resource::ReadbackHeapPtr& readbackHeap, D3D12_PLACED_SUBRESOURCE_FOOTPRINT& backBufferFootprint)
		{
			impl->saveBackBuffer(readbackHeap, backBufferFootprint);
		}

		void setDefRenderTexture()
		{
			impl->setDefRenderTexture();
		}

		void setRenderTexture(D3D12Resource::Texture* const renderTexture, const D3D12_CPU_DESCRIPTOR_HANDLE handle)
		{
			impl->setRenderTexture(renderTexture, handle);
		}

		void initializeResources()
		{
			impl->initializeResources();
		}

		void setDeltaTime(const float deltaTime)
		{
			Graphics::Internal::setDeltaTime(deltaTime);
		}

		void updateTimeElapsed()
		{
			Graphics::Internal::updateTimeElapsed();
		}

		void setFrameRate(const float frameRate)
		{
			Graphics::Internal::setFrameRate(frameRate);
		}

		void renderedFrameCountInc()
		{
			Graphics::Internal::renderedFrameCountInc();
		}
	}

	void submitCommandList(D3D12Core::CommandList* const commandList)
	{
		impl->submitCommandList(commandList);
	}

	AdapterVendor getVendor()
	{
		return impl->getVendor();
	}

	D3D12Resource::Texture* getRenderTexture()
	{
		return impl->getRenderTexture();
	}

	D3D12_CPU_DESCRIPTOR_HANDLE getBackBufferHandle()
	{
		return impl->getBackBufferHandle();
	}

	D3D12Core::CommandQueue* getCommandQueue()
	{
		return impl->getCommandQueue();
	}

	const Resource::ImmutableCBuffer* getEngineGlobalCBuffer()
	{
		return impl->getEngineGlobalCBuffer();
	}

	bool getDisplayImGuiSurface()
	{
		return impl->getDisplayImGuiSurface();
	}

	void toggleImGuiSurface()
	{
		impl->toggleImGuiSurface();
	}

	void toggleEngineImGuiSurface()
	{
		impl->toggleEngineImGuiSurface();
	}

	ImFont* getMediumFont()
	{
		return impl->getMediumFont();
	}

	ImFont* getLargeFont()
	{
		return impl->getLargeFont();
	}
}
