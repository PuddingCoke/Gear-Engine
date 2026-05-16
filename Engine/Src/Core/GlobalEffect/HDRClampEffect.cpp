#include<Gear/Core/GlobalEffect/HDRClampEffect.h>

#include<Gear/Core/GlobalEffect/Internal/HDRClampEffectInternal.h>

#include<Gear/CompiledShaders/HDRClampCS.h>

namespace
{
	struct HDRClampEffectPrivate
	{

		Gear::Core::D3D12Core::Shader* hdrClampShader;

		Gear::Core::D3D12Core::PipelineState* hdrClampState;

	}pvt;
}

void Gear::Core::GlobalEffect::HDRClampEffect::process(GraphicsContext* const context, Resource::TextureRenderView* const inOutTexture)
{
	if (inOutTexture->getTexture()->getFormat() == DXGI_FORMAT_R16G16B16A16_FLOAT)
	{
		context->setPipelineState(pvt.hdrClampState);

		context->setCSConstants({ inOutTexture->getUAVMipIndex(0) }, 0);

		context->transitionResources();

		context->dispatch(inOutTexture->getTexture()->getWidth() / 16 + 1, inOutTexture->getTexture()->getHeight() / 16 + 1, 1);

		context->uavBarrier({ inOutTexture->getTexture() });
	}
}

void Gear::Core::GlobalEffect::HDRClampEffect::Internal::initialize()
{
	pvt.hdrClampShader = new D3D12Core::Shader(g_HDRClampCSBytes, sizeof(g_HDRClampCSBytes));

	pvt.hdrClampState = PipelineStateBuilder::build(pvt.hdrClampShader);

	LOGSUCCESS(L"create", LogColor::brightMagenta, L"HDRClampEffect", LogColor::defaultColor, L"succeeded");
}

void Gear::Core::GlobalEffect::HDRClampEffect::Internal::release()
{
	if (pvt.hdrClampShader)
	{
		delete pvt.hdrClampShader;
	}

	if (pvt.hdrClampState)
	{
		delete pvt.hdrClampState;
	}
}
