#include<Gear/Effect/HDRClampEffect.h>

#include<Gear/Effect/Internal/HDRClampEffectInternal.h>

#include<Gear/CompiledShaders/HDRClampCS.h>

namespace Gear::Effect::HDRClampEffect
{
	class HDRClampEffectImpl
	{
	public:

		HDRClampEffectImpl();

		void process(GraphicsContext& refContext, Resource::RenderTextureView& inOutTexture);

	private:

		ComputeStatePtr hdrClampState;

	};

	HDRClampEffectImpl::HDRClampEffectImpl()
	{
		hdrClampState = PipelineStateBuilder::build(Shader::create(g_HDRClampCSBytes, sizeof(g_HDRClampCSBytes)));

		LOGSUCCESS() << "创建" << COLORIZESTRUCT(HDRClampEffect);
	}

	void HDRClampEffectImpl::process(GraphicsContext& refContext, Resource::RenderTextureView& inOutTexture)
	{
		GraphicsContext* const context = &refContext;

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

	namespace Internal
	{

		void initialize()
		{
			impl = makeUnique<HDRClampEffectImpl>();
		}

		void release()
		{
			impl.reset();
		}

	}

	void process(GraphicsContext& refContext, Resource::RenderTextureView& inOutTexture)
	{
		impl->process(refContext, inOutTexture);
	}
}
