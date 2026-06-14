#include<Gear/Effect/LatLongMapToCubeMapEffect.h>

#include<Gear/Effect/Internal/LatLongMapToCubeMapEffectInternal.h>

#include<Gear/Utils/Math.h>

#include<Gear/CompiledShaders/EquirectangularVS.h>

#include<Gear/CompiledShaders/EquirectangularPS.h>

namespace Gear::Effect::LatLongMapToCubeMapEffect
{
	namespace Internal
	{
		class LatLongMapToCubeMapEffectImpl
		{
		public:

			LatLongMapToCubeMapEffectImpl(ResourceManager* const resManager);

			void process(GraphicsContext& contextRef, RenderTextureView& inputTexture, RenderTextureView& outputTexture);

		private:

			ShaderPtr equirectangularVS;

			ShaderPtr equirectangularPS;

			PipelineStatePtr equirectangularR8State;

			PipelineStatePtr equirectangularR16State;

			PipelineStatePtr equirectangularR32State;

			ImmutableCBufferPtr matricesBuffer;

		};

		LatLongMapToCubeMapEffectImpl::LatLongMapToCubeMapEffectImpl(ResourceManager* const resManager)
		{
			equirectangularVS = Shader::create(g_EquirectangularVSBytes, sizeof(g_EquirectangularVSBytes));

			equirectangularPS = Shader::create(g_EquirectangularPSBytes, sizeof(g_EquirectangularPSBytes));

			{
				auto getBuilder = [&] {
					return PipelineStateBuilder()
						.setVS(*equirectangularVS)
						.setPS(*equirectangularPS)
						.setRasterizerState(PipelineStateHelper::rasterCullNone)
						.setBlendState(PipelineStateHelper::blendReplace)
						.setDepthStencilState(PipelineStateHelper::depthCompareNone)
						.setPrimitiveTopologyType(TOPOLOGY::TYPE::TRIANGLE);
					};

				equirectangularR8State = getBuilder().setRTVFormats({ FMT::RGBA8UN }).build();

				equirectangularR16State = getBuilder().setRTVFormats({ FMT::RGBA16F }).build();

				equirectangularR32State = getBuilder().setRTVFormats({ FMT::RGBA32F }).build();
			}

			{
				struct Matrices
				{
					DirectX::XMMATRIX matrices[6];
					DirectX::XMFLOAT4 padding[8];
				} matrices{};

				{
					const DirectX::XMVECTOR focusPoints[6] =
					{
						{1.0f,  0.0f,  0.0f},
						{-1.0f,  0.0f,  0.0f},
						{0.0f,  1.0f,  0.0f},
						{0.0f, -1.0f,  0.0f},
						{0.0f,  0.0f,  1.0f},
						{0.0f,  0.0f, -1.0f}
					};
					const DirectX::XMVECTOR upVectors[6] =
					{
						{0.0f, 1.0f,  0.0f},
						{0.0f, 1.0f,  0.0f},
						{0.0f,  0.0f,  -1.0f},
						{0.0f,  0.0f, 1.0f},
						{0.0f, 1.0f,  0.0f},
						{0.0f, 1.0f,  0.0f}
					};

					const DirectX::XMMATRIX projMatrix = DirectX::XMMatrixPerspectiveFovLH(Utils::Math::halfPi, 1.f, 0.1f, 10.f);

					for (uint32_t i = 0; i < 6; i++)
					{
						const DirectX::XMMATRIX viewMatrix = DirectX::XMMatrixLookAtLH({ 0.f,0.f,0.f }, focusPoints[i], upVectors[i]);
						const DirectX::XMMATRIX viewProj = DirectX::XMMatrixTranspose(viewMatrix * projMatrix);

						matrices.matrices[i] = viewProj;
					}
				}

				matricesBuffer = resManager->createImmutableCBuffer(sizeof(Matrices), &matrices, false);

				matricesBuffer->getBuffer()->setName(L"LatLongMap To Cubemap Matrices");
			}

			LOGSUCCESS(L"创建", LogColor::brightMagenta, TOWSTRING(LatLongMapToCubeMapEffect));
		}

		void LatLongMapToCubeMapEffectImpl::process(GraphicsContext& contextRef, RenderTextureView& inputTexture, RenderTextureView& outputTexture)
		{
			GraphicsContext* const context = &contextRef;

			switch (outputTexture.getTexture()->getFormat())
			{
			case FMT::RGBA8UN:
				context->setPipelineState(*equirectangularR8State);
				break;
			case FMT::RGBA16F:
				context->setPipelineState(*equirectangularR16State);
				break;
			case FMT::RGBA32F:
				context->setPipelineState(*equirectangularR32State);
				break;
			}

			context->setViewportSimple(outputTexture.getTexture()->getWidth(), outputTexture.getTexture()->getHeight());

			context->setPrimitiveTopology(TOPOLOGY::TRIANGLELIST);

			context->setRenderTargets({ outputTexture.getRTVMipHandle(0) }, {});

			context->setVSConstantBuffer(*matricesBuffer);

			SETCONSTS({
			context->setPSConstants({ inputTexture.getAllSRVIndex() }, co);
				});

			context->draw(36, 6, 0, 0);
		}

		UniquePtr<LatLongMapToCubeMapEffectImpl> impl;

		void initialize(ResourceManager* const resManager)
		{
			impl = makeUnique<LatLongMapToCubeMapEffectImpl>(resManager);
		}

		void release()
		{
			impl.reset();
		}
}

	void process(GraphicsContext& contextRef, RenderTextureView& inputTexture, RenderTextureView& outputTexture)
	{
		Internal::impl->process(contextRef, inputTexture, outputTexture);
	}
}

