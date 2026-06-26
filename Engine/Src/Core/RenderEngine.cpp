#include<Gear/Core/RenderEngine.h>

#include<Gear/Core/Internal/RenderEngineInternal.h>

#include<Gear/Core/Graphics.h>

#include<Gear/Core/MainCamera.h>

#include<Gear/Utils/Random.h>

#include<Gear/Utils/File.h>

#include<Gear/Core/D3D12Core/CommonShaderLayout.h>

#include<Gear/Core/Internal/GraphicsInternal.h>

#include<Gear/Core/Internal/GraphicsDeviceInternal.h>

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
	namespace Internal
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

				const D3D12Core::DescriptorHandle handle = GlobalDescriptorHeap::getResourceHeap()->allocStaticDescriptor(1);

				ImGui_ImplWin32_Init(hWnd);
				ImGui_ImplDX12_Init(GraphicsDevice::get(), Graphics::getFrameBufferCount(), Graphics::backBufferFormat,
					GlobalDescriptorHeap::getResourceHeap()->get(), handle.getCurrentCPUHandle(), handle.getCurrentGPUHandle());

				//显示输入法的待选框
				ImGui::GetMainViewport()->PlatformHandleRaw = (void*)hWnd;

				ImFontGlyphRangesBuilder builder;

				//加载常用汉字，GetGlyphRangesChineseSimplifiedCommon提供的汉字完全不够
				std::vector<uint8_t> chineseCharacters = Utils::File::readAllBinary(Utils::File::getRootFolder() + L"7000+symbols.txt");

				chineseCharacters.push_back('\0');

				builder.AddText(reinterpret_cast<const char*>(chineseCharacters.data()));

				//加载常用字符
				builder.AddRanges(io.Fonts->GetGlyphRangesDefault());

				ImVector<ImWchar> ranges;

				builder.BuildRanges(&ranges);

				//加载微软雅黑字体
				*mediumFont = io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/msyh.ttc", 18.f, nullptr, ranges.Data);

				*largeFont = io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/msyh.ttc", 24.f, nullptr, ranges.Data);

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

			ID3D12CommandQueue* getCommandQueue() const;

			void waitForCurrentFrame();

			void waitForNextFrame();

			void beginFrame();

			void endFrame();

			void present() const;

			void setDeltaTime(const float deltaTime) const;

			void updateTimeElapsed() const;

			void setDefRenderTexture();

			void setRenderTexture(D3D12Resource::Texture* const renderTexture, const D3D12_CPU_DESCRIPTOR_HANDLE handle);

			void initializeResources();

			void saveBackBuffer(D3D12Resource::ReadbackHeap* const readbackHeap);

			bool getDisplayImGuiSurface() const;

			void toggleImGuiSurface();

			void toggleEngineImGuiSurface();

			ImFont* getMediumFont() const;

			ImFont* getLargeFont() const;

		private:

			ComPtr<IDXGIAdapter4> getBestAdapterAndVendor(IDXGIFactory7* const factory);

			void processCommandLists();

			void updateDynamicCBuffers() const;

			void beginImGuiFrame() const;

			void drawImGuiFrame(D3D12Core::CommandList* const targetCommandList);

			UniquePtr<GraphicsDevice::Internal::InitializeToken> graphicsDeviceToken;

			ComPtr<ID3D12CommandQueue> commandQueue;

			ComPtr<ID3D12Fence> fence;

			UniquePtr<D3D12Core::CommandList> prepareCommandList;

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

			std::vector<D3D12Core::CommandList*> recordCommandLists;

			UniquePtr<uint64_t[]> fenceValues;

			HANDLE fenceEvent;

			UniquePtr<ImGuiToken> imGuiToken;

			ImFont* mediumFont;

			ImFont* largeFont;

			//引用
			D3D12Resource::Texture* renderTexture;

			std::mutex submitCommandListLock;

			int32_t syncInterval;

			D3D12Core::CommonShaderLayout::PerframeResource perframeResource;

			std::vector<D3D12_RESOURCE_BARRIER> resourceBarriers;

		};

		RenderEngineImpl::RenderEngineImpl(const uint32_t width, const uint32_t height, const HWND hWnd, const bool useSwapChainBuffer, const bool initializeImGuiSurface) :
			fenceEvent(CreateEvent(nullptr, FALSE, FALSE, nullptr)),
			vendor(AdapterVendor::UNKNOWN),
			initializeImGuiSurface(initializeImGuiSurface),
			displayImGuiSurface(false),
			displayEngineImGuiSurface(true),
			syncInterval(1),
			resManager(nullptr),
			perframeResource{}
		{
			//初始化一些渲染需要的信息，如width、height、frameIndex等
			Graphics::Internal::initialize(useSwapChainBuffer ? 3 : 1, width, height);

			ComPtr<IDXGIFactory7> factory;

#ifdef _DEBUG
			LOGENGINE(LogColor::brightGreen, L"开启", LogColor::brightMagenta, L"调试层");

			ComPtr<ID3D12Debug> debugController;

			D3D12GetDebugInterface(IID_PPV_ARGS(&debugController));

			debugController->EnableDebugLayer();

			CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&factory));
#else
			LOGENGINE(LogColor::brightRed, L"关闭", LogColor::brightMagenta, L"调试层");

			CreateDXGIFactory2(0, IID_PPV_ARGS(&factory));
#endif // _DEBUG

			//获取适配器
			ComPtr<IDXGIAdapter4> adapter = getBestAdapterAndVendor(factory.Get());

			//传入适配器，初始化图形设备(ID3D12Device)
			graphicsDeviceToken = makeUnique<GraphicsDevice::Internal::InitializeToken>(adapter.Get());

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

			//创建fence对象用于CPU和GPU之间的同步
			fenceValues = makeUnique<uint64_t[]>(Graphics::getFrameBufferCount());

			for (uint32_t i = 0; i < Graphics::getFrameBufferCount(); i++)
			{
				fenceValues[i] = 0;
			}

			GraphicsDevice::get()->CreateFence(fenceValues[Graphics::getFrameIndex()], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));

			fenceValues[Graphics::getFrameIndex()]++;

			//创建准备命令列表
			prepareCommandList = makeUnique<D3D12Core::CommandList>(D3D12_COMMAND_LIST_TYPE_DIRECT);

			//初始化线程局部资源
			renderThreadLocalToken = makeUnique<RenderThreadLocal::Internal::InitializeToken>();

			//初始化线程全局资源
			renderThreadGlobalToken = makeUnique<RenderThreadGlobal::Internal::InitializeToken>();

			//引擎需要使用一个动态常量缓冲为每一帧的渲染提供有用的信息
			engineGlobalCBuffer = ResourceManager::createDynamicCBuffer(sizeof(perframeResource));

			Graphics::Internal::setEngineGlobalCBuffer(engineGlobalCBuffer.get());

			//把准备命令列表推入容器中，因为资源的初始化可能需要动态常量缓冲
			//而动态常量缓冲更新的指令记录是由prepareCommandList负责的
			prepareCommandList->open();

			recordCommandLists.push_back(prepareCommandList.get());

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
				swapChainDesc.BufferCount = useSwapChainBuffer ? Graphics::getFrameBufferCount() : 2;
				swapChainDesc.Width = Graphics::getWidth();
				swapChainDesc.Height = Graphics::getHeight();
				swapChainDesc.Format = Graphics::backBufferFormat;
				swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
				swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
				swapChainDesc.SampleDesc.Count = 1;
				swapChainDesc.SampleDesc.Quality = 0;

				ComPtr<IDXGISwapChain1> swapChain1;

				factory->CreateSwapChainForHwnd(commandQueue.Get(), hWnd, &swapChainDesc, nullptr, nullptr, &swapChain1);

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
				LOGENGINE(LogColor::brightGreen, L"开启", LogColor::brightMagenta, L"ImGui");

				imGuiToken = makeUnique<ImGuiToken>(hWnd, &mediumFont, &largeFont);
			}
			else
			{
				LOGENGINE(LogColor::brightRed, L"关闭", LogColor::brightMagenta, L"ImGui");
			}

			//设置默认的2D投影矩阵
			MainCamera::setProj(DirectX::XMMatrixOrthographicOffCenterLH(0.f, static_cast<float>(Graphics::getWidth()), 0, static_cast<float>(Graphics::getHeight()), -1.f, 1.f));

			//设置默认的视图矩阵
			MainCamera::setView(DirectX::XMMatrixIdentity());
		}

		RenderEngineImpl::~RenderEngineImpl()
		{
			if (fenceEvent)
			{
				CloseHandle(fenceEvent);
			}
		}

		void RenderEngineImpl::submitCommandList(D3D12Core::CommandList* const commandList)
		{
			std::lock_guard<std::mutex> lockGuard(submitCommandListLock);

			D3D12Core::CommandList* const helperCommandList = recordCommandLists.back();

			if (commandList->hasPendingResource())
			{
				resourceBarriers.clear();

				commandList->flushPendingResources(resourceBarriers);

				if (helperCommandList != prepareCommandList.get())
				{
					helperCommandList->resourceBarrier(static_cast<uint32_t>(resourceBarriers.size()), resourceBarriers.data());
				}
				else
				{
					//尽量减少D3D12 API ResourceBarrier调用
					helperCommandList->pushResourceBarriers(resourceBarriers);
				}

				//有待定资源那么会需要更新资源的全局状态
				//因此最后会需要更新使用过的资源的全局状态
				commandList->flushReferredResources();
			}

			//不应该关闭准备命令列表，因为要用它来转变后备缓冲的状态，此外还要用它来记录动态常量缓冲更新指令。
			if (helperCommandList != prepareCommandList.get())
			{
				helperCommandList->close();
			}

			recordCommandLists.push_back(commandList);
		}

		AdapterVendor RenderEngineImpl::getVendor() const
		{
			return vendor;
		}

		D3D12Resource::Texture* RenderEngineImpl::getRenderTexture() const
		{
			return renderTexture;
		}

		ID3D12CommandQueue* RenderEngineImpl::getCommandQueue() const
		{
			return commandQueue.Get();
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

		void RenderEngineImpl::beginFrame()
		{
			beginImGuiFrame();

			prepareCommandList->open();

			recordCommandLists.push_back(prepareCommandList.get());

			//先获取可用的位置，供GraphicsContext在这一帧使用
			engineGlobalCBuffer->acquireDataPtr();

			//把后备缓冲转变到STATE_RENDER_TARGET，并暂存资源屏障
			prepareCommandList->trackAndSetResourceState(getRenderTexture(), D3D12Resource::D3D12_TRANSITION_ALL_MIPLEVELS, D3D12_RESOURCE_STATE_RENDER_TARGET);

			prepareCommandList->flushTransitionResources();
		}

		void RenderEngineImpl::endFrame()
		{
			if (displayImGuiSurface && displayEngineImGuiSurface)
			{
				ImGui::Begin("Frame Profile");
				ImGui::Text("TimeElapsed %.2f", Graphics::getTimeElapsed());
				ImGui::Text("FrameTime %.8f", ImGui::GetIO().DeltaTime * 1000.f);
				ImGui::Text("FrameRate %.1f", ImGui::GetIO().Framerate);
				ImGui::SliderInt("Sync Interval", &syncInterval, 0, 3);
				ImGui::End();

				Graphics::Internal::imGuiCall();
			}

			//把后备缓冲转变到STATE_RENDER_TARGET，并更新所有动态常量缓冲
			{
				updateDynamicCBuffers();

				//一些比较基础的信息的设置
				{
					perframeResource.deltaTime = Graphics::getDeltaTime();

					perframeResource.timeElapsed = Graphics::getTimeElapsed();

					perframeResource.uintSeed = Utils::Random::genUint();

					perframeResource.floatSeed = Utils::Random::genFloat();

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

				engineGlobalCBuffer->updateData(&perframeResource);
			}

			//使用最后一个命令列表做些收尾工作
			//如果需要ImGUI界面，那么把后备缓冲转变到STATE_PRESENT并绘制ImGUI帧
			{
				D3D12Core::CommandList* const finishCommandList = recordCommandLists.back();

				drawImGuiFrame(finishCommandList);

				finishCommandList->trackAndSetResourceState(getRenderTexture(), D3D12Resource::D3D12_TRANSITION_ALL_MIPLEVELS, D3D12_RESOURCE_STATE_PRESENT);

				finishCommandList->flushResourceBarriers();
			}

			processCommandLists();

			Graphics::Internal::renderedFrameCountInc();
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

		void RenderEngineImpl::setRenderTexture(D3D12Resource::Texture* const renderTexture, const D3D12_CPU_DESCRIPTOR_HANDLE handle)
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
			updateDynamicCBuffers();

			processCommandLists();

			//等待准备工作完成
			waitForCurrentFrame();

			RenderThreadLocal::Internal::flushCopiedResources();

			//清理静态资源管理器创建的临时资源
			resManager->cleanTransientResources();
		}

		void RenderEngineImpl::saveBackBuffer(D3D12Resource::ReadbackHeap* const readbackHeap)
		{
			D3D12_PLACED_SUBRESOURCE_FOOTPRINT bufferFootprint = {};

			bufferFootprint.Footprint.Width = getRenderTexture()->getWidth();

			bufferFootprint.Footprint.Height = getRenderTexture()->getHeight();

			bufferFootprint.Footprint.Depth = 1;

			bufferFootprint.Footprint.RowPitch = FMT::getByteSize(Graphics::backBufferFormat) * getRenderTexture()->getWidth();

			bufferFootprint.Footprint.Format = Graphics::backBufferFormat;

			const CD3DX12_TEXTURE_COPY_LOCATION copyDest(readbackHeap->getResource(), bufferFootprint);

			const CD3DX12_TEXTURE_COPY_LOCATION copySrc(getRenderTexture()->getResource(), 0);

			D3D12Core::CommandList* const lastCommandList = recordCommandLists.back();

			ID3D12GraphicsCommandList6* const id3d12LastCommandList = lastCommandList->get();

			lastCommandList->trackAndSetResourceState(getRenderTexture(), D3D12Resource::D3D12_TRANSITION_ALL_MIPLEVELS, D3D12_RESOURCE_STATE_COPY_SOURCE);

			lastCommandList->flushResourceBarriers();

			id3d12LastCommandList->CopyTextureRegion(&copyDest, 0, 0, 0, &copySrc, nullptr);
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
						vendor = AdapterVendor::NVIDIA;

						vendorName = L"NVIDIA";
					}
					else if (vendorID == 0x1002 || vendorID == 0x1022)
					{
						vendor = AdapterVendor::AMD;

						vendorName = L"AMD";
					}
					else if (vendorID == 0x163C || vendorID == 0x8086 || vendorID == 0x8087)
					{
						vendor = AdapterVendor::INTEL;

						vendorName = L"INTEL";
					}
					else
					{
						vendor = AdapterVendor::UNKNOWN;

						vendorName = L"UNKNOWN";
					}

					LOGENGINE(L"以下是适配器的相关信息");

					LOGENGINE(L"适配器名称", LogColor::brightMagenta, desc.Description);

					LOGENGINE(L"适配器生产商ID", IntegerMode::HEX, vendorID);

					LOGENGINE(L"适配器生产商", LogColor::brightMagenta, vendorName);

					LOGENGINE(L"适配器专有视频内存", static_cast<float>(desc.DedicatedVideoMemory) / 1024.f / 1024.f / 1024.f, L"GB");

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

		void RenderEngineImpl::updateDynamicCBuffers() const
		{
			DynamicCBufferManager::Internal::recordCommands(prepareCommandList.get());
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

		void RenderEngineImpl::drawImGuiFrame(D3D12Core::CommandList* const targetCommandList)
		{
			if (displayImGuiSurface)
			{
				targetCommandList->trackAndSetResourceState(getRenderTexture(), D3D12Resource::D3D12_TRANSITION_ALL_MIPLEVELS, D3D12_RESOURCE_STATE_RENDER_TARGET);

				targetCommandList->flushResourceBarriers();

				ImGui::Render();

				targetCommandList->setDefRenderTarget();

				ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), targetCommandList->get());
			}
		}

		UniquePtr<RenderEngineImpl> impl;

		void initialize(const uint32_t width, const uint32_t height, const HWND hwnd, const bool useSwapChainBuffer, const bool initializeImGuiSurface)
		{
			impl = makeUnique<RenderEngineImpl>(width, height, hwnd, useSwapChainBuffer, initializeImGuiSurface);
		}

		void release()
		{
			impl.reset();
		}

		void waitForCurrentFrame()
		{
			impl->waitForCurrentFrame();
		}

		void waitForNextFrame()
		{
			impl->waitForNextFrame();
		}

		void beginFrame()
		{
			impl->beginFrame();
		}

		void endFrame()
		{
			impl->endFrame();
		}

		void present()
		{
			impl->present();
		}

		void setDeltaTime(const float deltaTime)
		{
			impl->setDeltaTime(deltaTime);
		}

		void updateTimeElapsed()
		{
			impl->updateTimeElapsed();
		}

		void saveBackBuffer(D3D12Resource::ReadbackHeap* const readbackHeap)
		{
			impl->saveBackBuffer(readbackHeap);
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
	}

	void submitCommandList(D3D12Core::CommandList* const commandList)
	{
		Internal::impl->submitCommandList(commandList);
	}

	AdapterVendor getVendor()
	{
		return Internal::impl->getVendor();
	}

	D3D12Resource::Texture* getRenderTexture()
	{
		return Internal::impl->getRenderTexture();
	}

	ID3D12CommandQueue* getCommandQueue()
	{
		return Internal::impl->getCommandQueue();
	}

	bool getDisplayImGuiSurface()
	{
		return Internal::impl->getDisplayImGuiSurface();
	}

	void toggleImGuiSurface()
	{
		Internal::impl->toggleImGuiSurface();
	}

	void toggleEngineImGuiSurface()
	{
		Internal::impl->toggleEngineImGuiSurface();
	}

	ImFont* getMediumFont()
	{
		return Internal::impl->getMediumFont();
	}

	ImFont* getLargeFont()
	{
		return Internal::impl->getLargeFont();
	}
}