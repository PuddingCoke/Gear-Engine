#include<Gear/Core/GlobalEffect/HDRClampEffect.h>

#include<Gear/Core/GlobalEffect/Internal/HDRClampEffectInternal.h>

#include<Gear/CompiledShaders/HDRClampCS.h>

namespace Gear::Core::GlobalEffect::HDRClampEffect
{
	namespace Internal
	{
		struct HDRClampEffectImpl
		{

			UniquePtr<D3D12Core::Shader> hdrClampShader;

			UniquePtr<D3D12Core::PipelineState> hdrClampState;

		}impl;

		void initialize()
		{
			impl.hdrClampShader = D3D12Core::Shader::create(g_HDRClampCSBytes, sizeof(g_HDRClampCSBytes));

			impl.hdrClampState = PipelineStateBuilder::build(*impl.hdrClampShader);

			LOGSUCCESS(L"create", LogColor::brightMagenta, TOWSTRING(HDRClampEffect), LogColor::defaultColor, L"succeeded");
		}

		void release()
		{
			impl.hdrClampShader.reset();

			impl.hdrClampState.reset();
		}
	}

	void process(GraphicsContext& contextRef, Resource::RenderTextureView& inOutTexture)
	{
		GraphicsContext* const context = &contextRef;

		if (inOutTexture.getTexture()->getFormat() == FMT::RGBA16F)
		{
			context->setPipelineState(*Internal::impl.hdrClampState);

			SETCONSTS({
			context->setCSConstants({ inOutTexture.getUAVMipIndex(0) }, co);
				});

			context->dispatch(inOutTexture.getTexture()->getWidth() / 16 + 1, inOutTexture.getTexture()->getHeight() / 16 + 1, 1);

			context->uavBarrier({ inOutTexture.getTexture() });
		}
	}
}
