#include<Gear/Core/GlobalEffect/HDRClampEffect.h>

#include<Gear/Core/GlobalEffect/Internal/HDRClampEffectInternal.h>

#include<Gear/CompiledShaders/HDRClampCS.h>

namespace
{
	struct HDRClampEffectPrivate
	{

		UniquePtr<Gear::Core::D3D12Core::Shader> hdrClampShader;

		UniquePtr<Gear::Core::D3D12Core::PipelineState> hdrClampState;

	}pvt;
}

void Gear::Core::GlobalEffect::HDRClampEffect::process(GraphicsContext* const context, Resource::TextureRenderView& inOutTexture)
{
	if (inOutTexture.getTexture()->getFormat() == FMT::RGBA16F)
	{
		context->setPipelineState(*pvt.hdrClampState);

		context->setCSConstants({ inOutTexture.getUAVMipIndex(0) }, 0);

		context->dispatch(inOutTexture.getTexture()->getWidth() / 16 + 1, inOutTexture.getTexture()->getHeight() / 16 + 1, 1);

		context->uavBarrier({ inOutTexture.getTexture() });
	}
}

void Gear::Core::GlobalEffect::HDRClampEffect::Internal::initialize()
{
	pvt.hdrClampShader = D3D12Core::Shader::create(g_HDRClampCSBytes, sizeof(g_HDRClampCSBytes));

	pvt.hdrClampState = PipelineStateBuilder::build(*pvt.hdrClampShader);

	LOGSUCCESS(L"create", LogColor::brightMagenta, L"HDRClampEffect", LogColor::defaultColor, L"succeeded");
}

void Gear::Core::GlobalEffect::HDRClampEffect::Internal::release()
{
	pvt.hdrClampShader.reset();

	pvt.hdrClampState.reset();
}
