#include<Gear/Core/RenderEngine.h>

#include<Gear/Core/Internal/RenderEngineInternal.h>

#include<Gear/Core/DynamicCBufferManager.h>

#include<Gear/Core/GlobalRootSignature.h>

#include<Gear/Core/Graphics.h>

#include<Gear/Core/MainCamera.h>

#include<Gear/Utils/Random.h>

#include<Gear/Core/D3D12Core/CommonShaderLayout.h>

#include<Gear/Core/D3D12Core/Internal/DXCCompilerInternal.h>

#include<Gear/Core/Internal/GraphicsInternal.h>

#include<Gear/Core/Internal/GraphicsDeviceInternal.h>

#include<Gear/Core/Internal/GlobalDescriptorHeapInternal.h>

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

namespace
{
	class RenderEngineImpl
	{
	public:

		RenderEngineImpl() = delete;

		RenderEngineImpl(const RenderEngineImpl&) = delete;

		void operator=(const RenderEngineImpl&) = delete;

		RenderEngineImpl(const uint32_t width, const uint32_t height, const HWND hwnd, const bool useSwapChainBuffer, const bool initializeImGuiSurface);

		~RenderEngineImpl();

		void submitCommandList(Gear::Core::D3D12Core::CommandList* const commandList);

		Gear::Core::GPUVendor getVendor() const;

		Gear::Core::Resource::D3D12Resource::Texture* getRenderTexture() const;

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

		void setRenderTexture(Gear::Core::Resource::D3D12Resource::Texture* const renderTexture, const D3D12_CPU_DESCRIPTOR_HANDLE handle);

		void initializeResources();

		void saveBackBuffer(Gear::Core::Resource::D3D12Resource::ReadbackHeap* const readbackHeap);

	private:

		ComPtr<IDXGIAdapter4> getBestAdapterAndVendor(IDXGIFactory7* const factory);

		void processCommandLists();

		void updateConstantBuffer() const;

		void toggleImGuiSurface();

		void beginImGuiFrame() const;

		void drawImGuiFrame(Gear::Core::D3D12Core::CommandList* const targetCommandList);

		void createStaticResources();

		void releaseStaticResources();

		const bool initializeImGuiSurface;

		bool displayImGUISurface;

		Gear::Core::GPUVendor vendor;

		ComPtr<IDXGISwapChain4> swapChain;

		ComPtr<ID3D12CommandQueue> commandQueue;

		std::vector<Gear::Core::D3D12Core::CommandList*> recordCommandLists;

		ComPtr<ID3D12Fence> fence;

		uint64_t* fenceValues;

		HANDLE fenceEvent;

		Gear::Core::D3D12Core::CommandList* prepareCommandList;

		Gear::Core::Resource::D3D12Resource::Texture** backBufferTextures;

		D3D12_CPU_DESCRIPTOR_HANDLE* backBufferHandles;

		Gear::Core::Resource::D3D12Resource::Texture* renderTexture;

		std::mutex submitCommandListLock;

		int32_t syncInterval;

		Gear::Core::Resource::DynamicCBuffer* engineDefinedGlobalCBuffer;

		Gear::Core::ResourceManager* resManager;

		Gear::Core::D3D12Core::PerframeResource perframeResource;

	}*impl = nullptr;
}

RenderEngineImpl::RenderEngineImpl(const uint32_t width, const uint32_t height, const HWND hwnd, const bool useSwapChainBuffer, const bool initializeImGuiSurface) :
	backBufferTextures(nullptr),
	backBufferHandles(nullptr),
	fenceEvent(CreateEvent(nullptr, FALSE, FALSE, nullptr)),
	vendor(Gear::Core::GPUVendor::UNKNOWN),
	initializeImGuiSurface(initializeImGuiSurface),
	displayImGUISurface(false),
	syncInterval(1),
	resManager(nullptr),
	perframeResource{}
{
	Gear::Core::Graphics::Internal::initialize(useSwapChainBuffer ? 3 : 1, width, height);

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

	ComPtr<IDXGIAdapter4> adapter = getBestAdapterAndVendor(factory.Get());

	Gear::Core::GraphicsDevice::Internal::initialize(adapter.Get());

	Gear::Core::GraphicsDevice::Internal::checkFeatureSupport();

	{
		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_DISABLE_GPU_TIMEOUT;

		Gear::Core::GraphicsDevice::get()->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue));

		commandQueue->SetName(L"Graphics Command Queue");
	}

	Gear::Core::D3D12Core::DXCCompiler::Internal::initialize();

	Gear::Core::GlobalShader::Internal::initialize();

	Gear::Core::GlobalDescriptorHeap::Internal::initializeGlobalDescriptorHeaps();

	Gear::Core::GlobalDescriptorHeap::Internal::initializeLocalDescriptorHeaps();

	Gear::Core::GlobalRootSignature::Internal::initialize();

	Gear::Core::DynamicCBufferManager::Internal::initialize();

	//设置默认的2D投影矩阵
	Gear::Core::MainCamera::setProj(DirectX::XMMatrixOrthographicOffCenterLH(0.f, static_cast<float>(Gear::Core::Graphics::getWidth()), 0, static_cast<float>(Gear::Core::Graphics::getHeight()), -1.f, 1.f));

	//设置默认的视图矩阵
	Gear::Core::MainCamera::setView(DirectX::XMMatrixIdentity());

	{
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
		swapChainDesc.BufferCount = useSwapChainBuffer ? Gear::Core::Graphics::getFrameBufferCount() : 2;
		swapChainDesc.Width = Gear::Core::Graphics::getWidth();
		swapChainDesc.Height = Gear::Core::Graphics::getHeight();
		swapChainDesc.Format = Gear::Core::Graphics::backBufferFormat;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;

		ComPtr<IDXGISwapChain1> swapChain1;

		factory->CreateSwapChainForHwnd(commandQueue.Get(), hwnd, &swapChainDesc, nullptr, nullptr, &swapChain1);

		factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER);

		swapChain1.As(&swapChain);
	}

	fenceValues = new uint64_t[Gear::Core::Graphics::getFrameBufferCount()];

	for (uint32_t i = 0; i < Gear::Core::Graphics::getFrameBufferCount(); i++)
	{
		fenceValues[i] = 0;
	}

	Gear::Core::GraphicsDevice::get()->CreateFence(fenceValues[Gear::Core::Graphics::getFrameIndex()], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));

	fenceValues[Gear::Core::Graphics::getFrameIndex()]++;

	prepareCommandList = new Gear::Core::D3D12Core::CommandList(D3D12_COMMAND_LIST_TYPE_DIRECT);

	engineDefinedGlobalCBuffer = Gear::Core::ResourceManager::createDynamicCBuffer(sizeof(perframeResource)).release();

	Gear::Core::Graphics::Internal::setEngineDefinedGlobalCBuffer(engineDefinedGlobalCBuffer);

	//确保准备命令列表总是处于容器第一个位置
	begin();

	if (useSwapChainBuffer)
	{
		Gear::Core::D3D12Core::DescriptorHandle descriptorHandle = Gear::Core::GlobalDescriptorHeap::getRenderTargetHeap()->allocStaticDescriptor(Gear::Core::Graphics::getFrameBufferCount());

		backBufferTextures = new Gear::Core::Resource::D3D12Resource::Texture * [Gear::Core::Graphics::getFrameBufferCount()];

		backBufferHandles = new D3D12_CPU_DESCRIPTOR_HANDLE[Gear::Core::Graphics::getFrameBufferCount()];

		for (uint32_t i = 0; i < Gear::Core::Graphics::getFrameBufferCount(); i++)
		{
			ComPtr<ID3D12Resource> texture;

			swapChain->GetBuffer(i, IID_PPV_ARGS(&texture));

			Gear::Core::GraphicsDevice::get()->CreateRenderTargetView(texture.Get(), nullptr, descriptorHandle.getCurrentCPUHandle());

			backBufferHandles[i] = descriptorHandle.getCurrentCPUHandle();

			descriptorHandle.move();

			backBufferTextures[i] = new Gear::Core::Resource::D3D12Resource::Texture(texture, true, D3D12_RESOURCE_STATE_PRESENT);
		}
	}

	if (initializeImGuiSurface)
	{
		LOGENGINE(L"enable ImGui");

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		(void)io;

		ImGui::StyleColorsDark();

		const Gear::Core::D3D12Core::DescriptorHandle handle = Gear::Core::GlobalDescriptorHeap::getResourceHeap()->allocStaticDescriptor(1);

		ImGui_ImplWin32_Init(hwnd);
		ImGui_ImplDX12_Init(Gear::Core::GraphicsDevice::get(), Gear::Core::Graphics::getFrameBufferCount(), Gear::Core::Graphics::backBufferFormat,
			Gear::Core::GlobalDescriptorHeap::getResourceHeap()->get(), handle.getCurrentCPUHandle(), handle.getCurrentGPUHandle());
	}
	else
	{
		LOGENGINE(L"disable ImGui");
	}

	CHECKERROR(CoInitializeEx(0, COINIT_MULTITHREADED));

	resManager = new Gear::Core::ResourceManager();

	createStaticResources();

	submitCommandList(resManager->getCommandList());
}

RenderEngineImpl::~RenderEngineImpl()
{
	releaseStaticResources();

	if (resManager)
	{
		delete resManager;
	}

	CoUninitialize();

	if (initializeImGuiSurface)
	{
		ImGui_ImplDX12_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	}

	if (backBufferTextures)
	{
		for (uint32_t i = 0; i < Gear::Core::Graphics::getFrameBufferCount(); i++)
		{
			delete backBufferTextures[i];
		}

		delete[] backBufferTextures;
	}

	if (backBufferHandles)
	{
		delete[] backBufferHandles;
	}

	if (engineDefinedGlobalCBuffer)
	{
		delete engineDefinedGlobalCBuffer;
	}

	if (prepareCommandList)
	{
		delete prepareCommandList;
	}

	fence = nullptr;

	if (fenceValues)
	{
		delete[] fenceValues;
	}

	swapChain = nullptr;

	Gear::Core::DynamicCBufferManager::Internal::release();

	Gear::Core::GlobalRootSignature::Internal::release();

	Gear::Core::GlobalDescriptorHeap::Internal::releaseLocalDescriptorHeaps();

	Gear::Core::GlobalDescriptorHeap::Internal::releaseGlobalDescriptorHeaps();

	Gear::Core::GlobalShader::Internal::release();

	Gear::Core::D3D12Core::DXCCompiler::Internal::release();

	commandQueue = nullptr;

	Gear::Core::GraphicsDevice::Internal::release();

	if (fenceEvent)
	{
		CloseHandle(fenceEvent);
	}
}

void RenderEngineImpl::submitCommandList(Gear::Core::D3D12Core::CommandList* const commandList)
{
	std::lock_guard<std::mutex> lockGuard(submitCommandListLock);

	std::vector<D3D12_RESOURCE_BARRIER> barriers;

	commandList->solvePendingBarriers(barriers);

	Gear::Core::D3D12Core::CommandList* const helperCommandList = recordCommandLists.back();

	if (barriers.size() > 0)
	{
		helperCommandList->resourceBarrier(static_cast<uint32_t>(barriers.size()), barriers.data());
	}

	//不应该关闭准备命令列表，因为要用它来转变后备缓冲的状态，此外还要用它来记录动态常量缓冲更新指令。
	if (helperCommandList != prepareCommandList)
	{
		helperCommandList->close();
	}

	recordCommandLists.push_back(commandList);

	commandList->updateReferredSharedResourceStates();
}

Gear::Core::GPUVendor RenderEngineImpl::getVendor() const
{
	return vendor;
}

Gear::Core::Resource::D3D12Resource::Texture* RenderEngineImpl::getRenderTexture() const
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
	commandQueue->Signal(fence.Get(), fenceValues[Gear::Core::Graphics::getFrameIndex()]);

	fence->SetEventOnCompletion(fenceValues[Gear::Core::Graphics::getFrameIndex()], fenceEvent);

	WaitForSingleObjectEx(fenceEvent, INFINITE, FALSE);

	fenceValues[Gear::Core::Graphics::getFrameIndex()]++;
}

void RenderEngineImpl::waitForNextFrame()
{
	const uint64_t currentFenceValue = fenceValues[Gear::Core::Graphics::getFrameIndex()];

	commandQueue->Signal(fence.Get(), currentFenceValue);

	Gear::Core::Graphics::Internal::setFrameIndex(swapChain->GetCurrentBackBufferIndex());

	if (fence->GetCompletedValue() < fenceValues[Gear::Core::Graphics::getFrameIndex()])
	{
		fence->SetEventOnCompletion(fenceValues[Gear::Core::Graphics::getFrameIndex()], fenceEvent);

		WaitForSingleObjectEx(fenceEvent, INFINITE, FALSE);
	}

	fenceValues[Gear::Core::Graphics::getFrameIndex()] = currentFenceValue + 1;
}

void RenderEngineImpl::begin()
{
	beginImGuiFrame();

	prepareCommandList->open();

	recordCommandLists.push_back(prepareCommandList);

	Gear::Core::Graphics::Internal::renderedFrameCountInc();

	engineDefinedGlobalCBuffer->acquireDataPtr();
}

void RenderEngineImpl::end()
{
	if (displayImGUISurface)
	{
		ImGui::Begin("Frame Profile");
		ImGui::Text("TimeElapsed %.2f", Gear::Core::Graphics::getTimeElapsed());
		ImGui::Text("FrameTime %.8f", ImGui::GetIO().DeltaTime * 1000.f);
		ImGui::Text("FrameRate %.1f", ImGui::GetIO().Framerate);
		ImGui::SliderInt("Sync Interval", &syncInterval, 0, 3);
		ImGui::End();

		Gear::Core::Graphics::Internal::imGUICall();
	}

	//把后备缓冲转变到STATE_RENDER_TARGET，并更新所有动态常量缓冲
	{
		prepareCommandList->trackAndSetResourceState(getRenderTexture(), Gear::Core::Resource::D3D12Resource::D3D12_TRANSITION_ALL_MIPLEVELS, D3D12_RESOURCE_STATE_RENDER_TARGET);

		updateConstantBuffer();

		//一些比较基础的信息的设置
		{
			perframeResource.deltaTime = Gear::Core::Graphics::getDeltaTime();

			perframeResource.timeElapsed = Gear::Core::Graphics::getTimeElapsed();

			perframeResource.uintSeed = Gear::Utils::Random::genUint();

			perframeResource.floatSeed = Gear::Utils::Random::genFloat();

			perframeResource.screenSize = DirectX::XMFLOAT2(
				static_cast<float>(Gear::Core::Graphics::getWidth()),
				static_cast<float>(Gear::Core::Graphics::getHeight()));

			perframeResource.screenTexelSize = DirectX::XMFLOAT2(
				1.f / perframeResource.screenSize.x,
				1.f / perframeResource.screenSize.y);
		}

		//主相机相关信息的设置
		{
			perframeResource.prevViewProj = perframeResource.viewProj;

			perframeResource.proj = DirectX::XMMatrixTranspose(Gear::Core::MainCamera::getProj());

			perframeResource.view = DirectX::XMMatrixTranspose(Gear::Core::MainCamera::getView());

			perframeResource.viewProj = DirectX::XMMatrixTranspose(Gear::Core::MainCamera::getView() * Gear::Core::MainCamera::getProj());

			//逆的转置的转置等于没有转置
			perframeResource.normalMatrix = DirectX::XMMatrixInverse(nullptr, Gear::Core::MainCamera::getView());

			DirectX::XMStoreFloat4(&perframeResource.eyePos, Gear::Core::MainCamera::getEyePos());
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
		Gear::Core::D3D12Core::CommandList* const finishCommandList = recordCommandLists.back();

		drawImGuiFrame(finishCommandList);

		finishCommandList->trackAndSetResourceState(getRenderTexture(), Gear::Core::Resource::D3D12Resource::D3D12_TRANSITION_ALL_MIPLEVELS, D3D12_RESOURCE_STATE_PRESENT);

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
	Gear::Core::Graphics::Internal::setDeltaTime(deltaTime);
}

void RenderEngineImpl::updateTimeElapsed() const
{
	Gear::Core::Graphics::Internal::updateTimeElapsed();
}

void RenderEngineImpl::setDefRenderTexture()
{
	setRenderTexture(backBufferTextures[Gear::Core::Graphics::getFrameIndex()], backBufferHandles[Gear::Core::Graphics::getFrameIndex()]);
}

void RenderEngineImpl::setRenderTexture(Gear::Core::Resource::D3D12Resource::Texture* const renderTexture, const D3D12_CPU_DESCRIPTOR_HANDLE handle)
{
	//状态转变
	this->renderTexture = renderTexture;

	//渲染目标
	Gear::Core::Graphics::Internal::setBackBufferHandle(handle);
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

void RenderEngineImpl::saveBackBuffer(Gear::Core::Resource::D3D12Resource::ReadbackHeap* const readbackHeap)
{
	D3D12_PLACED_SUBRESOURCE_FOOTPRINT bufferFootprint = {};

	bufferFootprint.Footprint.Width = getRenderTexture()->getWidth();

	bufferFootprint.Footprint.Height = getRenderTexture()->getHeight();

	bufferFootprint.Footprint.Depth = 1;

	bufferFootprint.Footprint.RowPitch = Gear::Core::FMT::getByteSize(Gear::Core::Graphics::backBufferFormat) * getRenderTexture()->getWidth();

	bufferFootprint.Footprint.Format = Gear::Core::Graphics::backBufferFormat;

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
				vendor = Gear::Core::GPUVendor::NVIDIA;

				vendorName = L"NVIDIA";
			}
			else if (vendorID == 0x1002 || vendorID == 0x1022)
			{
				vendor = Gear::Core::GPUVendor::AMD;

				vendorName = L"AMD";
			}
			else if (vendorID == 0x163C || vendorID == 0x8086 || vendorID == 0x8087)
			{
				vendor = Gear::Core::GPUVendor::INTEL;

				vendorName = L"INTEL";
			}
			else
			{
				vendor = Gear::Core::GPUVendor::UNKNOWN;

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

	for (const Gear::Core::D3D12Core::CommandList* const commandList : recordCommandLists)
	{
		commandLists.push_back(commandList->get());
	}

	recordCommandLists.clear();

	commandQueue->ExecuteCommandLists(static_cast<uint32_t>(commandLists.size()), commandLists.data());
}

void RenderEngineImpl::updateConstantBuffer() const
{
	Gear::Core::DynamicCBufferManager::Internal::recordCommands(prepareCommandList);
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

void RenderEngineImpl::drawImGuiFrame(Gear::Core::D3D12Core::CommandList* const targetCommandList)
{
	if (displayImGUISurface)
	{
		ImGui::Render();

		//targetCommandList->setDescriptorHeap(Gear::Core::GlobalDescriptorHeap::getResourceHeap(), Gear::Core::GlobalDescriptorHeap::getSamplerHeap());

		targetCommandList->setDefRenderTarget();

		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), targetCommandList->get());
	}
}

void RenderEngineImpl::createStaticResources()
{
	Gear::Core::GraphicsContext* const context = resManager->getGraphicsContext();

	context->begin();

	Gear::Core::GlobalEffect::BackBufferBlitEffect::Internal::initialize();

	Gear::Core::GlobalEffect::HDRClampEffect::Internal::initialize();

	Gear::Core::GlobalEffect::LatLongMapToCubeMapEffect::Internal::initialize(resManager);

	Gear::Core::GlobalEffect::ToneMapEffect::Internal::initialize();

	Gear::Core::GlobalEffect::GammaCorrectEffect::Internal::initialize();
}

void RenderEngineImpl::releaseStaticResources()
{
	Gear::Core::GlobalEffect::BackBufferBlitEffect::Internal::release();

	Gear::Core::GlobalEffect::HDRClampEffect::Internal::release();

	Gear::Core::GlobalEffect::LatLongMapToCubeMapEffect::Internal::release();

	Gear::Core::GlobalEffect::ToneMapEffect::Internal::release();

	Gear::Core::GlobalEffect::GammaCorrectEffect::Internal::release();
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
	impl = new RenderEngineImpl(width, height, hwnd, useSwapChainBuffer, initializeImGuiSurface);
}

void Gear::Core::RenderEngine::Internal::release()
{
	if (impl)
	{
		delete impl;
	}
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