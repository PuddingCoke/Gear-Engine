#include<Gear/Core/RenderEngine.h>

#include<Gear/Core/Internal/RenderEngineInternal.h>

#include<Gear/Core/Graphics.h>

#include<Gear/Core/MainCamera.h>

#include<Gear/Utils/Random.h>

#include<Gear/Core/D3D12Core/CommonShaderLayout.h>

#include<Gear/Core/D3D12Core/Internal/DXCCompilerInternal.h>

#include<Gear/Core/Internal/GraphicsInternal.h>

#include<Gear/Core/Internal/GraphicsDeviceInternal.h>

#include<Gear/Core/Internal/GlobalDescriptorHeapInternal.h>

#include<Gear/Core/Internal/LocalDescriptorHeapInternal.h>

#include<Gear/Core/Internal/GlobalRootSignatureInternal.h>

#include<Gear/Core/Internal/GlobalShaderInternal.h>

#include<Gear/Core/Internal/DynamicCBufferManagerInternal.h>

#include<Gear/Core/GlobalEffect/Internal/BackBufferBlitEffectInternal.h>

#include<Gear/Core/GlobalEffect/Internal/HDRClampEffectInternal.h>

#include<Gear/Core/GlobalEffect/Internal/LatLongMapToCubeMapEffectInternal.h>

#include<Gear/Core/GlobalEffect/Internal/ToneMapEffectInternal.h>

#include<Gear/Core/GlobalEffect/Internal/GammaCorrectEffectInternal.h>

#include<ImGUI/imgui.h>

#include<ImGUI/imgui_impl_win32.h>

#include<ImGUI/imgui_impl_dx12.h>

namespace Gear::Core::RenderEngine::Internal
{
	class RenderEngineImpl
	{
	public:

		RenderEngineImpl() = delete;

		RenderEngineImpl(const RenderEngineImpl&) = delete;

		void operator=(const RenderEngineImpl&) = delete;

		RenderEngineImpl(const uint32_t width, const uint32_t height, const HWND hwnd, const bool useSwapChainBuffer, const bool initializeImGuiSurface);

		~RenderEngineImpl();

		void submitCommandList(D3D12Core::CommandList* const commandList);

		GPUVendor getVendor() const;

		Resource::D3D12Resource::Texture* getRenderTexture() const;

		ID3D12CommandQueue* getCommandQueue() const;

		bool getDisplayImGuiSurface() const;

		void waitForCurrentFrame();

		void waitForNextFrame();

		void begin();

		void end();

		void present() const;

		void setDeltaTime(const float deltaTime) const;

		void updateTimeElapsed() const;

		void setDefRenderTexture();

		void setRenderTexture(Resource::D3D12Resource::Texture* const renderTexture, const D3D12_CPU_DESCRIPTOR_HANDLE handle);

		void initializeResources();

		void saveBackBuffer(Resource::D3D12Resource::ReadbackHeap* const readbackHeap);

	private:

		ComPtr<IDXGIAdapter4> getBestAdapterAndVendor(IDXGIFactory7* const factory);

		void processCommandLists();

		void updateConstantBuffer() const;

		void toggleImGuiSurface();

		void beginImGuiFrame() const;

		void drawImGuiFrame(D3D12Core::CommandList* const targetCommandList);

		void createGlobalEffects();

		void releaseGlobalEffects();

		const bool initializeImGuiSurface;

		bool displayImGUISurface;

		GPUVendor vendor;

		ComPtr<IDXGISwapChain4> swapChain;

		ComPtr<ID3D12CommandQueue> commandQueue;

		std::vector<D3D12Core::CommandList*> recordCommandLists;

		ComPtr<ID3D12Fence> fence;

		UniquePtr<uint64_t[]> fenceValues;

		HANDLE fenceEvent;

		UniquePtr<D3D12Core::CommandList> prepareCommandList;

		std::vector<UniquePtr<Resource::D3D12Resource::Texture>> backBufferTextures;

		UniquePtr<D3D12_CPU_DESCRIPTOR_HANDLE[]> backBufferHandles;

		//引用
		Resource::D3D12Resource::Texture* renderTexture;

		std::mutex submitCommandListLock;

		int32_t syncInterval;

		UniquePtr<Resource::DynamicCBuffer> engineDefinedGlobalCBuffer;

		UniquePtr<ResourceManager> resManager;

		D3D12Core::PerframeResource perframeResource;

	};

	RenderEngineImpl::RenderEngineImpl(const uint32_t width, const uint32_t height, const HWND hwnd, const bool useSwapChainBuffer, const bool initializeImGuiSurface) :
		fenceEvent(CreateEvent(nullptr, FALSE, FALSE, nullptr)),
		vendor(GPUVendor::UNKNOWN),
		initializeImGuiSurface(initializeImGuiSurface),
		displayImGUISurface(false),
		syncInterval(1),
		resManager(nullptr),
		perframeResource{}
	{
		//初始化一些渲染需要的信息，如width、height、frameIndex等
		Graphics::Internal::initialize(useSwapChainBuffer ? 3 : 1, width, height);

		ComPtr<IDXGIFactory7> factory;

#ifdef _DEBUG
		LOGENGINE(LogColor::brightGreen, L"enable", LogColor::defaultColor, L"debug layer");

		ComPtr<ID3D12Debug> debugController;

		D3D12GetDebugInterface(IID_PPV_ARGS(&debugController));

		debugController->EnableDebugLayer();

		CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&factory));
#else
		LOGENGINE(LogColor::brightRed, L"disable", LogColor::defaultColor, L"debug layer");

		CreateDXGIFactory2(0, IID_PPV_ARGS(&factory));
#endif // _DEBUG

		//获取适配器
		ComPtr<IDXGIAdapter4> adapter = getBestAdapterAndVendor(factory.Get());

		//传入适配器，初始化图形设备(ID3D12Device)
		GraphicsDevice::Internal::initialize(adapter.Get());

		//检查并输出一些特性的支持情况
		//不支持Shader Model 6.6或有类型UAV读取会报错
		GraphicsDevice::Internal::checkFeatureSupport();

		//初始化图形设备后创建命令队列
		{
			D3D12_COMMAND_QUEUE_DESC queueDesc = {};
			queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
			queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_DISABLE_GPU_TIMEOUT;

			GraphicsDevice::get()->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue));

			commandQueue->SetName(L"Graphics Command Queue");
		}

		//创建命令队列后创建交换链
		{
			DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
			swapChainDesc.BufferCount = useSwapChainBuffer ? Graphics::getFrameBufferCount() : 2;
			swapChainDesc.Width = Graphics::getWidth();
			swapChainDesc.Height = Graphics::getHeight();
			swapChainDesc.Format = Graphics::backBufferFormat;
			swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
			swapChainDesc.SampleDesc.Count = 1;
			swapChainDesc.SampleDesc.Quality = 0;

			ComPtr<IDXGISwapChain1> swapChain1;

			factory->CreateSwapChainForHwnd(commandQueue.Get(), hwnd, &swapChainDesc, nullptr, nullptr, &swapChain1);

			factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER);

			swapChain1.As(&swapChain);
		}

		//创建fence对象用于CPU和GPU之间的同步
		fenceValues = makeUnique<uint64_t[]>(Graphics::getFrameBufferCount());

		for (uint32_t i = 0; i < Graphics::getFrameBufferCount(); i++)
		{
			fenceValues[i] = 0;
		}

		GraphicsDevice::get()->CreateFence(fenceValues[Graphics::getFrameIndex()], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));

		fenceValues[Graphics::getFrameIndex()]++;
		/////////////////////////////////

		//创建准备命令列表
		prepareCommandList = makeUnique<D3D12Core::CommandList>(D3D12_COMMAND_LIST_TYPE_DIRECT);

		//设置默认的2D投影矩阵
		MainCamera::setProj(DirectX::XMMatrixOrthographicOffCenterLH(0.f, static_cast<float>(Graphics::getWidth()), 0, static_cast<float>(Graphics::getHeight()), -1.f, 1.f));

		//设置默认的视图矩阵
		MainCamera::setView(DirectX::XMMatrixIdentity());

		//渲染相关的基础设施的初始化
		D3D12Core::DXCCompiler::Internal::initialize();

		GlobalShader::Internal::initialize();

		GlobalDescriptorHeap::Internal::initialize();

		LocalDescriptorHeap::Internal::initialize();

		GlobalRootSignature::Internal::initialize();

		DynamicCBufferManager::Internal::initialize();
		////////////////////////

		//引擎需要使用一个动态常量缓冲为每一帧的渲染提供有用的信息
		engineDefinedGlobalCBuffer = ResourceManager::createDynamicCBuffer(sizeof(perframeResource));

		Graphics::Internal::setEngineDefinedGlobalCBuffer(engineDefinedGlobalCBuffer.get());
		//////////////////////////////////////////////////

		//把准备命令列表推入容器中，因为资源的初始化可能需要动态常量缓冲
		//而动态常量缓冲更新的指令记录是由prepareCommandList负责的
		begin();

		//如果需要使用交换链的后备缓冲的话，那么需要为交换链的缓冲创建RTV
		if (useSwapChainBuffer)
		{
			D3D12Core::DescriptorHandle descriptorHandle = LocalDescriptorHeap::getRenderTargetHeap()->allocStaticDescriptor(Graphics::getFrameBufferCount());

			backBufferHandles = makeUnique<D3D12_CPU_DESCRIPTOR_HANDLE[]>(Graphics::getFrameBufferCount());

			for (uint32_t i = 0; i < Graphics::getFrameBufferCount(); i++)
			{
				ComPtr<ID3D12Resource> texture;

				swapChain->GetBuffer(i, IID_PPV_ARGS(&texture));

				GraphicsDevice::get()->CreateRenderTargetView(texture.Get(), nullptr, descriptorHandle.getCurrentCPUHandle());

				backBufferHandles[i] = descriptorHandle.getCurrentCPUHandle();

				descriptorHandle.move();

				//后备缓冲的初态为D3D12_RESOURCE_STATE_PRESENT
				backBufferTextures.emplace_back(makeUnique<Resource::D3D12Resource::Texture>(texture, true, D3D12_RESOURCE_STATE_PRESENT));
			}
		}

		//初始化ImGUI，如果有需要
		if (initializeImGuiSurface)
		{
			LOGENGINE(L"enable ImGui");

			IMGUI_CHECKVERSION();
			ImGui::CreateContext();
			ImGuiIO& io = ImGui::GetIO();
			(void)io;

			ImGui::StyleColorsDark();

			const D3D12Core::DescriptorHandle handle = GlobalDescriptorHeap::getResourceHeap()->allocStaticDescriptor(1);

			ImGui_ImplWin32_Init(hwnd);
			ImGui_ImplDX12_Init(GraphicsDevice::get(), Graphics::getFrameBufferCount(), Graphics::backBufferFormat,
				GlobalDescriptorHeap::getResourceHeap()->get(), handle.getCurrentCPUHandle(), handle.getCurrentGPUHandle());
		}
		else
		{
			LOGENGINE(L"disable ImGui");
		}

		CHECKERROR(CoInitializeEx(0, COINIT_MULTITHREADED));

		resManager = makeUnique<ResourceManager>();

		createGlobalEffects();

		submitCommandList(resManager->getCommandList());
	}

	RenderEngineImpl::~RenderEngineImpl()
	{
		if (initializeImGuiSurface)
		{
			ImGui_ImplDX12_Shutdown();
			ImGui_ImplWin32_Shutdown();
			ImGui::DestroyContext();
		}

		releaseGlobalEffects();

		resManager.reset();

		CoUninitialize();

		backBufferTextures.clear();

		engineDefinedGlobalCBuffer.reset();

		prepareCommandList.reset();

		fence = nullptr;

		swapChain = nullptr;

		DynamicCBufferManager::Internal::release();

		GlobalRootSignature::Internal::release();

		GlobalDescriptorHeap::Internal::release();

		LocalDescriptorHeap::Internal::release();

		GlobalShader::Internal::release();

		D3D12Core::DXCCompiler::Internal::release();

		commandQueue = nullptr;

		GraphicsDevice::Internal::release();

		if (fenceEvent)
		{
			CloseHandle(fenceEvent);
		}
	}

	void RenderEngineImpl::submitCommandList(D3D12Core::CommandList* const commandList)
	{
		std::lock_guard<std::mutex> lockGuard(submitCommandListLock);

		std::vector<D3D12_RESOURCE_BARRIER> barriers;

		commandList->solvePendingBarriers(barriers);

		D3D12Core::CommandList* const helperCommandList = recordCommandLists.back();

		if (barriers.size() > 0)
		{
			helperCommandList->resourceBarrier(static_cast<uint32_t>(barriers.size()), barriers.data());
		}

		//不应该关闭准备命令列表，因为要用它来转变后备缓冲的状态，此外还要用它来记录动态常量缓冲更新指令。
		if (helperCommandList != prepareCommandList.get())
		{
			helperCommandList->close();
		}

		recordCommandLists.push_back(commandList);

		commandList->updateReferredSharedResourceStates();
	}

	GPUVendor RenderEngineImpl::getVendor() const
	{
		return vendor;
	}

	Resource::D3D12Resource::Texture* RenderEngineImpl::getRenderTexture() const
	{
		return renderTexture;
	}

	ID3D12CommandQueue* RenderEngineImpl::getCommandQueue() const
	{
		return commandQueue.Get();
	}

	bool RenderEngineImpl::getDisplayImGuiSurface() const
	{
		return displayImGUISurface;
	}

	void RenderEngineImpl::waitForCurrentFrame()
	{
		commandQueue->Signal(fence.Get(), fenceValues[Graphics::getFrameIndex()]);

		fence->SetEventOnCompletion(fenceValues[Graphics::getFrameIndex()], fenceEvent);

		WaitForSingleObjectEx(fenceEvent, INFINITE, FALSE);

		fenceValues[Graphics::getFrameIndex()]++;
	}

	void RenderEngineImpl::waitForNextFrame()
	{
		const uint64_t currentFenceValue = fenceValues[Graphics::getFrameIndex()];

		commandQueue->Signal(fence.Get(), currentFenceValue);

		Graphics::Internal::setFrameIndex(swapChain->GetCurrentBackBufferIndex());

		if (fence->GetCompletedValue() < fenceValues[Graphics::getFrameIndex()])
		{
			fence->SetEventOnCompletion(fenceValues[Graphics::getFrameIndex()], fenceEvent);

			WaitForSingleObjectEx(fenceEvent, INFINITE, FALSE);
		}

		fenceValues[Graphics::getFrameIndex()] = currentFenceValue + 1;
	}

	void RenderEngineImpl::begin()
	{
		beginImGuiFrame();

		prepareCommandList->open();

		recordCommandLists.push_back(prepareCommandList.get());

		Graphics::Internal::renderedFrameCountInc();

		engineDefinedGlobalCBuffer->acquireDataPtr();
	}

	void RenderEngineImpl::end()
	{
		if (displayImGUISurface)
		{
			ImGui::Begin("Frame Profile");
			ImGui::Text("TimeElapsed %.2f", Graphics::getTimeElapsed());
			ImGui::Text("FrameTime %.8f", ImGui::GetIO().DeltaTime * 1000.f);
			ImGui::Text("FrameRate %.1f", ImGui::GetIO().Framerate);
			ImGui::SliderInt("Sync Interval", &syncInterval, 0, 3);
			ImGui::End();

			Graphics::Internal::imGUICall();
		}

		//把后备缓冲转变到STATE_RENDER_TARGET，并更新所有动态常量缓冲
		{
			prepareCommandList->trackAndSetResourceState(getRenderTexture(), Resource::D3D12Resource::D3D12_TRANSITION_ALL_MIPLEVELS, D3D12_RESOURCE_STATE_RENDER_TARGET);

			updateConstantBuffer();

			//一些比较基础的信息的设置
			{
				perframeResource.deltaTime = Graphics::getDeltaTime();

				perframeResource.timeElapsed = Graphics::getTimeElapsed();

				perframeResource.uintSeed = Gear::Utils::Random::genUint();

				perframeResource.floatSeed = Gear::Utils::Random::genFloat();

				perframeResource.screenSize = DirectX::XMFLOAT2(
					static_cast<float>(Graphics::getWidth()),
					static_cast<float>(Graphics::getHeight()));

				perframeResource.screenTexelSize = DirectX::XMFLOAT2(
					1.f / perframeResource.screenSize.x,
					1.f / perframeResource.screenSize.y);
			}

			//主相机相关信息的设置
			{
				perframeResource.prevViewProj = perframeResource.viewProj;

				perframeResource.proj = DirectX::XMMatrixTranspose(MainCamera::getProj());

				perframeResource.view = DirectX::XMMatrixTranspose(MainCamera::getView());

				perframeResource.viewProj = DirectX::XMMatrixTranspose(MainCamera::getView() * MainCamera::getProj());

				//逆的转置的转置等于没有转置
				perframeResource.normalMatrix = DirectX::XMMatrixInverse(nullptr, MainCamera::getView());

				DirectX::XMStoreFloat4(&perframeResource.eyePos, MainCamera::getEyePos());
			}
			//关于为什么要转置我找到了一篇有关的文章
			//https://www.douduck08.com/zh-tw/why-dx11-need-matrix-transpose-before-cbuffer-mapping/
			//这里简要说一下，其实和矩阵如何被解释有关，矩阵实际上是以一维数组的形式被存储的
			//DirectXMath默认其为Row Major，而HLSL默认其为Column Major
			//在DirectXMath中我们一般使用DirectX::XMVector4Transform，它背后的数学运算是 vec*matrix
			//如果数据原封不动上传到显存上，那么这个矩阵会被HLSL用另一种方式来解释，我们因此需要的数学运算是 matrix*vec，即mul(matrix,vec)
			//然而，mul(vec,matrix)是有一些性能优势的，为了利用这个性能优势，矩阵在上传前要被转置

			engineDefinedGlobalCBuffer->updateData(&perframeResource);
		}

		//使用最后一个命令列表做些收尾工作
		//如有需要，则绘制ImGui帧
		//把后备缓冲转变到STATE_PRESENT
		{
			D3D12Core::CommandList* const finishCommandList = recordCommandLists.back();

			drawImGuiFrame(finishCommandList);

			finishCommandList->trackAndSetResourceState(getRenderTexture(), Resource::D3D12Resource::D3D12_TRANSITION_ALL_MIPLEVELS, D3D12_RESOURCE_STATE_PRESENT);

			finishCommandList->transitionResources();
		}

		processCommandLists();
	}

	void RenderEngineImpl::present() const
	{
		swapChain->Present(static_cast<uint32_t>(syncInterval), 0);
	}

	void RenderEngineImpl::setDeltaTime(const float deltaTime) const
	{
		Graphics::Internal::setDeltaTime(deltaTime);
	}

	void RenderEngineImpl::updateTimeElapsed() const
	{
		Graphics::Internal::updateTimeElapsed();
	}

	void RenderEngineImpl::setDefRenderTexture()
	{
		setRenderTexture(backBufferTextures[Graphics::getFrameIndex()].get(), backBufferHandles[Graphics::getFrameIndex()]);
	}

	void RenderEngineImpl::setRenderTexture(Resource::D3D12Resource::Texture* const renderTexture, const D3D12_CPU_DESCRIPTOR_HANDLE handle)
	{
		//状态转变
		this->renderTexture = renderTexture;

		//渲染目标
		Graphics::Internal::setBackBufferHandle(handle);
	}

	void RenderEngineImpl::initializeResources()
	{
		//如果有需要，那么开启ImGui
		toggleImGuiSurface();

		//更新动态常量缓冲，因为资源创建可能会需要动态常量缓冲
		updateConstantBuffer();

		processCommandLists();

		//等待准备工作完成
		waitForCurrentFrame();

		//清理静态资源管理器创建的临时资源
		resManager->cleanTransientResources();
	}

	void RenderEngineImpl::saveBackBuffer(Resource::D3D12Resource::ReadbackHeap* const readbackHeap)
	{
		D3D12_PLACED_SUBRESOURCE_FOOTPRINT bufferFootprint = {};

		bufferFootprint.Footprint.Width = getRenderTexture()->getWidth();

		bufferFootprint.Footprint.Height = getRenderTexture()->getHeight();

		bufferFootprint.Footprint.Depth = 1;

		bufferFootprint.Footprint.RowPitch = FMT::getByteSize(Graphics::backBufferFormat) * getRenderTexture()->getWidth();

		bufferFootprint.Footprint.Format = Graphics::backBufferFormat;

		const CD3DX12_TEXTURE_COPY_LOCATION copyDest(readbackHeap->getResource(), bufferFootprint);

		const CD3DX12_TEXTURE_COPY_LOCATION copySrc(getRenderTexture()->getResource(), 0);

		ID3D12GraphicsCommandList6* const lastCommandList = recordCommandLists.back()->get();

		D3D12_RESOURCE_BARRIER barrier = {};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.pResource = getRenderTexture()->getResource();
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_SOURCE;

		lastCommandList->ResourceBarrier(1, &barrier);

		lastCommandList->CopyTextureRegion(&copyDest, 0, 0, 0, &copySrc, nullptr);

		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_SOURCE;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

		lastCommandList->ResourceBarrier(1, &barrier);
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

			if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
			{
				continue;
			}

			if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
			{
				const uint32_t vendorID = desc.VendorId;

				std::wstring vendorName;

				if (vendorID == 0x10DE)
				{
					vendor = GPUVendor::NVIDIA;

					vendorName = L"NVIDIA";
				}
				else if (vendorID == 0x1002 || vendorID == 0x1022)
				{
					vendor = GPUVendor::AMD;

					vendorName = L"AMD";
				}
				else if (vendorID == 0x163C || vendorID == 0x8086 || vendorID == 0x8087)
				{
					vendor = GPUVendor::INTEL;

					vendorName = L"INTEL";
				}
				else
				{
					vendor = GPUVendor::UNKNOWN;

					vendorName = L"UNKNOWN";
				}

				LOGENGINE(L"following are information about selected GPU");

				LOGENGINE(L"GPU Name", LogColor::brightMagenta, desc.Description);

				LOGENGINE(L"GPU Vendor ID", IntegerMode::HEX, vendorID);

				LOGENGINE(L"GPU Vendor Name", LogColor::brightMagenta, vendorName);

				LOGENGINE(L"GPU Dedicated Memory", static_cast<float>(desc.DedicatedVideoMemory) / 1024.f / 1024.f / 1024.f, L"gigabytes");

				break;
			}
		}

		return adapter;
	}

	void RenderEngineImpl::processCommandLists()
	{
		recordCommandLists.front()->close();

		recordCommandLists.back()->close();

		std::vector<ID3D12CommandList*> commandLists;

		for (const D3D12Core::CommandList* const commandList : recordCommandLists)
		{
			commandLists.push_back(commandList->get());
		}

		recordCommandLists.clear();

		commandQueue->ExecuteCommandLists(static_cast<uint32_t>(commandLists.size()), commandLists.data());
	}

	void RenderEngineImpl::updateConstantBuffer() const
	{
		DynamicCBufferManager::Internal::recordCommands(prepareCommandList.get());
	}

	void RenderEngineImpl::toggleImGuiSurface()
	{
		if (initializeImGuiSurface)
		{
			displayImGUISurface = !displayImGUISurface;
		}
	}

	void RenderEngineImpl::beginImGuiFrame() const
	{
		if (displayImGUISurface)
		{
			ImGui_ImplDX12_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();
		}
	}

	void RenderEngineImpl::drawImGuiFrame(D3D12Core::CommandList* const targetCommandList)
	{
		if (displayImGUISurface)
		{
			ImGui::Render();

			//targetCommandList->setDescriptorHeap(GlobalDescriptorHeap::getResourceHeap(), GlobalDescriptorHeap::getSamplerHeap());

			targetCommandList->setDefRenderTarget();

			ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), targetCommandList->get());
		}
	}

	void RenderEngineImpl::createGlobalEffects()
	{
		GraphicsContext* const context = resManager->getGraphicsContext();

		context->begin();

		GlobalEffect::BackBufferBlitEffect::Internal::initialize();

		GlobalEffect::HDRClampEffect::Internal::initialize();

		GlobalEffect::LatLongMapToCubeMapEffect::Internal::initialize(resManager.get());

		GlobalEffect::ToneMapEffect::Internal::initialize();

		GlobalEffect::GammaCorrectEffect::Internal::initialize();
	}

	void RenderEngineImpl::releaseGlobalEffects()
	{
		GlobalEffect::BackBufferBlitEffect::Internal::release();

		GlobalEffect::HDRClampEffect::Internal::release();

		GlobalEffect::LatLongMapToCubeMapEffect::Internal::release();

		GlobalEffect::ToneMapEffect::Internal::release();

		GlobalEffect::GammaCorrectEffect::Internal::release();
	}
}

namespace
{
	UniquePtr<Gear::Core::RenderEngine::Internal::RenderEngineImpl> impl;
}

void Gear::Core::RenderEngine::submitCommandList(Gear::Core::D3D12Core::CommandList* const commandList)
{
	impl->submitCommandList(commandList);
}

Gear::Core::GPUVendor Gear::Core::RenderEngine::getVendor()
{
	return impl->getVendor();
}

Gear::Core::Resource::D3D12Resource::Texture* Gear::Core::RenderEngine::getRenderTexture()
{
	return impl->getRenderTexture();
}

ID3D12CommandQueue* Gear::Core::RenderEngine::getCommandQueue()
{
	return impl->getCommandQueue();
}

bool Gear::Core::RenderEngine::getDisplayImGuiSurface()
{
	return impl->getDisplayImGuiSurface();
}

void Gear::Core::RenderEngine::Internal::initialize(const uint32_t width, const uint32_t height, const HWND hwnd, const bool useSwapChainBuffer, const bool initializeImGuiSurface)
{
	impl = makeUnique<RenderEngineImpl>(width, height, hwnd, useSwapChainBuffer, initializeImGuiSurface);
}

void Gear::Core::RenderEngine::Internal::release()
{
	impl.reset();
}

void Gear::Core::RenderEngine::Internal::waitForCurrentFrame()
{
	impl->waitForCurrentFrame();
}

void Gear::Core::RenderEngine::Internal::waitForNextFrame()
{
	impl->waitForNextFrame();
}

void Gear::Core::RenderEngine::Internal::begin()
{
	impl->begin();
}

void Gear::Core::RenderEngine::Internal::end()
{
	impl->end();
}

void Gear::Core::RenderEngine::Internal::present()
{
	impl->present();
}

void Gear::Core::RenderEngine::Internal::setDeltaTime(const float deltaTime)
{
	impl->setDeltaTime(deltaTime);
}

void Gear::Core::RenderEngine::Internal::updateTimeElapsed()
{
	impl->updateTimeElapsed();
}

void Gear::Core::RenderEngine::Internal::saveBackBuffer(Resource::D3D12Resource::ReadbackHeap* const readbackHeap)
{
	impl->saveBackBuffer(readbackHeap);
}

void Gear::Core::RenderEngine::Internal::setDefRenderTexture()
{
	impl->setDefRenderTexture();
}

void Gear::Core::RenderEngine::Internal::setRenderTexture(Resource::D3D12Resource::Texture* const renderTexture, const D3D12_CPU_DESCRIPTOR_HANDLE handle)
{
	impl->setRenderTexture(renderTexture, handle);
}

void Gear::Core::RenderEngine::Internal::initializeResources()
{
	impl->initializeResources();
}