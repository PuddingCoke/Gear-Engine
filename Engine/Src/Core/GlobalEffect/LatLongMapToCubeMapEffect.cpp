#include<Gear/Core/GlobalEffect/LatLongMapToCubeMapEffect.h>

#include<Gear/Core/GlobalEffect/Internal/LatLongMapToCubeMapEffectInternal.h>

#include<Gear/Utils/Math.h>

#include<Gear/CompiledShaders/EquirectangularVS.h>

#include<Gear/CompiledShaders/EquirectangularPS.h>

namespace Gear::Core::GlobalEffect::LatLongMapToCubeMapEffect
{
	namespace Internal
	{
		struct LatLongMapToCubeMapEffectImpl
		{

			D3D12Core::ShaderPtr equirectangularVS;

			D3D12Core::ShaderPtr equirectangularPS;

			D3D12Core::PipelineStatePtr equirectangularR8State;

			D3D12Core::PipelineStatePtr equirectangularR16State;

			D3D12Core::PipelineStatePtr equirectangularR32State;

			Resource::ImmutableCBufferPtr matricesBuffer;

		}impl;

		void initialize(ResourceManager* const resManager)
		{
			impl.equirectangularVS = D3D12Core::Shader::create(g_EquirectangularVSBytes, sizeof(g_EquirectangularVSBytes));

			impl.equirectangularPS = D3D12Core::Shader::create(g_EquirectangularPSBytes, sizeof(g_EquirectangularPSBytes));

			{
				auto getBuilder = [] {
					return PipelineStateBuilder()
						.setVS(*impl.equirectangularVS)
						.setPS(*impl.equirectangularPS)
						.setRasterizerState(PipelineStateHelper::rasterCullNone)
						.setBlendState(PipelineStateHelper::blendReplace)
						.setDepthStencilState(PipelineStateHelper::depthCompareNone)
						.setPrimitiveTopologyType(TOPOLOGY::TYPE::TRIANGLE);
					};

				impl.equirectangularR8State = getBuilder().setRTVFormats({ FMT::RGBA8UN }).build();

				impl.equirectangularR16State = getBuilder().setRTVFormats({ FMT::RGBA16F }).build();

				impl.equirectangularR32State = getBuilder().setRTVFormats({ FMT::RGBA32F }).build();

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

				impl.matricesBuffer = resManager->createImmutableCBuffer(sizeof(Matrices), &matrices, false);

				impl.matricesBuffer->getBuffer()->setName(L"LatLongMap To Cubemap Matrices");
			}

			LOGSUCCESS(L"创建", LogColor::brightMagenta, TOWSTRING(LatLongMapToCubeMapEffect));
		}

		void release()
		{
			impl.equirectangularVS.reset();

			impl.equirectangularPS.reset();

			impl.equirectangularR8State.reset();

			impl.equirectangularR16State.reset();

			impl.equirectangularR32State.reset();

			impl.matricesBuffer.reset();
		}
	}

	void process(GraphicsContext& contextRef, Resource::RenderTextureView& inputTexture, Resource::RenderTextureView& outputTexture)
	{
		GraphicsContext* const context = &contextRef;

		switch (outputTexture.getTexture()->getFormat())
		{
		case FMT::RGBA8UN:
			context->setPipelineState(*Internal::impl.equirectangularR8State);
			break;
		case FMT::RGBA16F:
			context->setPipelineState(*Internal::impl.equirectangularR16State);
			break;
		case FMT::RGBA32F:
			context->setPipelineState(*Internal::impl.equirectangularR32State);
			break;
		}

		context->setViewportSimple(outputTexture.getTexture()->getWidth(), outputTexture.getTexture()->getHeight());

		context->setPrimitiveTopology(TOPOLOGY::TRIANGLELIST);

		context->setRenderTargets({ outputTexture.getRTVMipHandle(0) }, {});

		context->setVSConstantBuffer(*Internal::impl.matricesBuffer);

		SETCONSTS({
		context->setPSConstants({ inputTexture.getAllSRVIndex() }, co);
			});

		context->draw(36, 6, 0, 0);
	}
}

