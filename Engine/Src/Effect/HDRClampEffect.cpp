#include<Gear/Effect/HDRClampEffect.h>

#include<Gear/Effect/Internal/HDRClampEffectInternal.h>

#include<Gear/CompiledShaders/HDRClampCS.h>

namespace Gear::Effect::HDRClampEffect
{
	namespace Internal
	{
		class HDRClampEffectImpl
		{
		public:

			HDRClampEffectImpl();

			void process(GraphicsContext& contextRef, Resource::RenderTextureView& inOutTexture);

		private:

			ShaderPtr hdrClampShader;

			PipelineStatePtr hdrClampState;

		};

		HDRClampEffectImpl::HDRClampEffectImpl()
		{
			hdrClampShader = Shader::create(g_HDRClampCSBytes, sizeof(g_HDRClampCSBytes));

			hdrClampState = PipelineStateBuilder::build(*hdrClampShader);

			LOGSUCCESS(L"创建", LogColor::brightMagenta, TOWSTRING(HDRClampEffect));
		}

		void HDRClampEffectImpl::process(GraphicsContext& contextRef, Resource::RenderTextureView& inOutTexture)
		{
			GraphicsContext* const context = &contextRef;

			if (inOutTexture.getTexture()->getFormat() == FMT::RGBA16F)
			{
				context->setPipelineState(*hdrClampState);

				SETCONSTS({
				context->setCSConstants({ inOutTexture.getUAVMipIndex(0) }, co);
					});

				context->dispatchDim(inOutTexture.get3Dimension());
			}
		}

		UniquePtr<HDRClampEffectImpl> impl;

		void initialize()
		{
			impl = makeUnique<HDRClampEffectImpl>();
		}

		void release()
		{
			impl.reset();
		}
	}

	void process(GraphicsContext& contextRef, Resource::RenderTextureView& inOutTexture)
	{
		Internal::impl->process(contextRef, inOutTexture);
	}
}
