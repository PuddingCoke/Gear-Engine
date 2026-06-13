#include<Gear/Core/GlobalEffect/HDRClampEffect.h>

#include<Gear/Core/GlobalEffect/Internal/HDRClampEffectInternal.h>

#include<Gear/CompiledShaders/HDRClampCS.h>

namespace Gear::Core::GlobalEffect::HDRClampEffect
{
	namespace Internal
	{
		struct HDRClampEffectImpl
		{

			D3D12Core::ShaderPtr hdrClampShader;

			D3D12Core::PipelineStatePtr hdrClampState;

		}impl;

		void initialize()
		{
			impl.hdrClampShader = D3D12Core::Shader::create(g_HDRClampCSBytes, sizeof(g_HDRClampCSBytes));

			impl.hdrClampState = PipelineStateBuilder::build(*impl.hdrClampShader);

			LOGSUCCESS(L"创建", LogColor::brightMagenta, TOWSTRING(HDRClampEffect));
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

			context->dispatchDim(inOutTexture.get3Dimension());
		}
	}
}
