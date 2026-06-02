#include<Gear/Core/Effect/HBAOPlusEffect.h>

#include<Gear/Core/MainCamera.h>

#include<Gear/Core/RenderEngine.h>

UniquePtr<Gear::Core::Effect::HBAOPlusEffect> Gear::Core::Effect::HBAOPlusEffect::create(GraphicsContext* const context, const uint32_t width, const uint32_t height)
{
	return makeUnique<HBAOPlusEffect>(context, width, height);
}

Gear::Core::Effect::HBAOPlusEffect::HBAOPlusEffect(GraphicsContext* const context, const uint32_t width, const uint32_t height) :
	EffectBase(context, width, height, FMT::R32F), aoParameters{}
{
	GFSDK_SSAO_CustomHeap customHeap;

	customHeap.new_ = ::operator new;

	customHeap.delete_ = ::operator delete;

	D3D12Core::DescriptorHandle srvUAVCBVHandle = GlobalDescriptorHeap::getResourceHeap()->allocStaticDescriptor(GFSDK_SSAO_NUM_DESCRIPTORS_CBV_SRV_UAV_HEAP_D3D12);

	D3D12Core::DescriptorHandle rtvHandle = LocalDescriptorHeap::getRenderTargetHeap()->allocStaticDescriptor(GFSDK_SSAO_NUM_DESCRIPTORS_RTV_HEAP_D3D12);

	GFSDK_SSAO_DescriptorHeaps_D3D12 descriptorHeaps;

	descriptorHeaps.CBV_SRV_UAV.pDescHeap = srvUAVCBVHandle.getDescriptorHeap()->get();

	descriptorHeaps.CBV_SRV_UAV.BaseIndex = srvUAVCBVHandle.getCurrentIndex();

	descriptorHeaps.RTV.pDescHeap = rtvHandle.getDescriptorHeap()->get();

	descriptorHeaps.RTV.BaseIndex = rtvHandle.getCurrentIndex();

	GFSDK_SSAO_Status status = GFSDK_SSAO_CreateContext_D3D12(GraphicsDevice::get(), 1, descriptorHeaps, &aoContext, &customHeap);

	if (status == GFSDK_SSAO_OK)
	{
		LOGENGINE(L"initialize", LogColor::brightMagenta, L"HBAOPlusEffect", LogColor::defaultColor, L"successfully");
	}
	else
	{
		LOGERROR(L"initialize HBAOPlusEffect failed, fail code:", static_cast<uint32_t>(status));
	}

	aoParameters.Radius = 2.f;
	aoParameters.Bias = 0.2f;
	aoParameters.PowerExponent = 2.f;
	aoParameters.Blur.Enable = true;
	aoParameters.Blur.Sharpness = 32.f;
	aoParameters.Blur.Radius = GFSDK_SSAO_BLUR_RADIUS_4;
	aoParameters.DepthStorage = GFSDK_SSAO_FP32_VIEW_DEPTHS;
	aoParameters.EnableDualLayerAO = false;

	outputTexture->getTexture()->setName(L"AO Texture");
}

Gear::Core::Effect::HBAOPlusEffect::~HBAOPlusEffect()
{
	if (aoContext)
		aoContext->Release();
}

void Gear::Core::Effect::HBAOPlusEffect::imGUICall()
{
	ImGui::Begin("HBAOPlusEffect");
	ImGui::SliderFloat("AO Radius", &aoParameters.Radius, 0.f, 16.f);
	ImGui::SliderFloat("AO Bias", &aoParameters.Bias, 0.f, 1.f);
	ImGui::SliderFloat("AO Power Exponent", &aoParameters.PowerExponent, 0.f, 16.f);
	ImGui::SliderFloat("AO Blur Sharpness", &aoParameters.Blur.Sharpness, 0.f, 64.f);
	ImGui::End();
}

Gear::Core::Resource::TextureRenderView* Gear::Core::Effect::HBAOPlusEffect::process(Resource::TextureDepthView& depthTexture, Resource::TextureRenderView& gNormal)
{
	GFSDK_SSAO_InputData_D3D12 inputData = {};

	const DirectX::XMMATRIX projMatrix = MainCamera::getProj();

	const DirectX::XMMATRIX viewMatrix = MainCamera::getView();

	inputData.DepthData.DepthTextureType = GFSDK_SSAO_HARDWARE_DEPTHS;
	inputData.DepthData.FullResDepthTextureSRV.pResource = depthTexture.getTexture()->getResource();
	inputData.DepthData.FullResDepthTextureSRV.GpuHandle = depthTexture.getDepthMipGPUHandle(0).ptr;
	inputData.DepthData.ProjectionMatrix.Data = GFSDK_SSAO_Float4x4((const GFSDK_SSAO_FLOAT*)&projMatrix);
	inputData.DepthData.ProjectionMatrix.Layout = GFSDK_SSAO_ROW_MAJOR_ORDER;
	inputData.DepthData.MetersToViewSpaceUnits = 1.f;

	inputData.NormalData.Enable = true;
	inputData.NormalData.FullResNormalTextureSRV.pResource = gNormal.getTexture()->getResource();
	inputData.NormalData.FullResNormalTextureSRV.GpuHandle = gNormal.getSRVMipGPUHandle(0).ptr;
	inputData.NormalData.WorldToViewMatrix.Data = GFSDK_SSAO_Float4x4((const GFSDK_SSAO_FLOAT*)&viewMatrix);
	inputData.NormalData.WorldToViewMatrix.Layout = GFSDK_SSAO_ROW_MAJOR_ORDER;

	GFSDK_SSAO_RenderMask renderMask = GFSDK_SSAO_RENDER_AO;

	GFSDK_SSAO_Output_D3D12 output;

	GFSDK_SSAO_RenderTargetView_D3D12 rtv{};

	auto renderTargetDesc = outputTexture->getRTVMipHandle(0);

	rtv.pResource = outputTexture->getTexture()->getResource();

	rtv.CpuHandle = renderTargetDesc.rtvHandle.ptr;

	output.pRenderTargetView = &rtv;

	// Renders SSAO.
	//
	// Remarks:
	//    * Allocates internal D3D render targets on first use, and re-allocates them when the viewport dimensions change.
	//    * Setting RenderMask = GFSDK_SSAO_RENDER_DEBUG_NORMAL_Z can be useful to visualize the normals used for the AO rendering.
	//    * The input depth & normal textures are assumed to have state D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE.
	//    * The output AO render target is assumed to have state D3D12_RESOURCE_STATE_RENDER_TARGET.

	D3D12Core::CommandList* const commandList = context->getCommandList();

	commandList->trackAndSetResourceState(depthTexture.getTexture(), 0, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

	commandList->trackAndSetResourceState(gNormal.getTexture(), 0, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

	commandList->trackAndSetResourceState(outputTexture->getTexture(), 0, D3D12_RESOURCE_STATE_RENDER_TARGET);

	commandList->transitionResources();

	const GFSDK_SSAO_Status status = aoContext->RenderAO(RenderEngine::getCommandQueue(), commandList->get(), inputData, aoParameters, output, renderMask);

	if (status != GFSDK_SSAO_OK)
	{
		LOGERROR(L"renderAO method failed, fail code:", static_cast<uint32_t>(status));
	}

	//使用Nsight调试后发现RenderAO不会恢复图形相关的状态
	//这会让引擎内部追踪的一些图形状态失效
	//所以得在这里重置内部追踪的图形状态
	context->resetPipelineState();
	
	context->resetTrackedGraphicsStates();

	return outputTexture.get();
}
