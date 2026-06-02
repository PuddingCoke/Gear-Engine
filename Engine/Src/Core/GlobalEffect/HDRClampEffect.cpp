#include<Gear/Core/GlobalEffect/HDRClampEffect.h>

#include<Gear/Core/GlobalEffect/Internal/HDRClampEffectInternal.h>

#include<Gear/CompiledShaders/HDRClampCS.h>

namespace
{
	struct HDRClampEffectImpl
	{

		UniquePtr<Gear::Core::D3D12Core::Shader> hdrClampShader;

		UniquePtr<Gear::Core::D3D12Core::PipelineState> hdrClampState;

	}impl;
}

void Gear::Core::GlobalEffect::HDRClampEffect::process(GraphicsContext* const context, Resource::TextureRenderView& inOutTexture)
{
	if (inOutTexture.getTexture()->getFormat() == FMT::RGBA16F)
	{
		context->setPipelineState(*impl.hdrClampState);

		context->setCSConstants({ inOutTexture.getUAVMipIndex(0) }, 0);

		context->dispatch(inOutTexture.getTexture()->getWidth() / 16 + 1, inOutTexture.getTexture()->getHeight() / 16 + 1, 1);

		context->uavBarrier({ inOutTexture.getTexture() });
	}
}

void Gear::Core::GlobalEffect::HDRClampEffect::Internal::initialize()
{
	impl.hdrClampShader = D3D12Core::Shader::create(g_HDRClampCSBytes, sizeof(g_HDRClampCSBytes));

	impl.hdrClampState = PipelineStateBuilder::build(*impl.hdrClampShader);

	LOGSUCCESS(L"create", LogColor::brightMagenta, L"HDRClampEffect", LogColor::defaultColor, L"succeeded");
}

void Gear::Core::GlobalEffect::HDRClampEffect::Internal::release()
{
	impl.hdrClampShader.reset();

	impl.hdrClampState.reset();
}
