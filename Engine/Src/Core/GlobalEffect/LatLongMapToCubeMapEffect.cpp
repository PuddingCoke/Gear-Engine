#include<Gear/Core/GlobalEffect/LatLongMapToCubeMapEffect.h>

#include<Gear/Core/GlobalEffect/Internal/LatLongMapToCubeMapEffectInternal.h>

#include<Gear/Utils/Math.h>

#include<Gear/CompiledShaders/EquirectangularVS.h>

#include<Gear/CompiledShaders/EquirectangularPS.h>

namespace
{
	struct LatLongMapToCubeMapEffectPrivate
	{

		UniquePtr<Gear::Core::D3D12Core::Shader> equirectangularVS;

		UniquePtr<Gear::Core::D3D12Core::Shader> equirectangularPS;

		UniquePtr<Gear::Core::D3D12Core::PipelineState> equirectangularR8State;

		UniquePtr<Gear::Core::D3D12Core::PipelineState> equirectangularR16State;

		UniquePtr<Gear::Core::D3D12Core::PipelineState> equirectangularR32State;

		UniquePtr<Gear::Core::Resource::ImmutableCBuffer> matricesBuffer;

	}pvt;
}

void Gear::Core::GlobalEffect::LatLongMapToCubeMapEffect::process(GraphicsContext* const context, Resource::TextureRenderView& inputTexture, Resource::TextureRenderView& outputTexture)
{
	switch (outputTexture.getTexture()->getFormat())
	{
	case FMT::RGBA8UN:
		context->setPipelineState(*pvt.equirectangularR8State);
		break;
	case FMT::RGBA16F:
		context->setPipelineState(*pvt.equirectangularR16State);
		break;
	case FMT::RGBA32F:
		context->setPipelineState(*pvt.equirectangularR32State);
		break;
	}

	context->setViewportSimple(outputTexture.getTexture()->getWidth(), outputTexture.getTexture()->getHeight());

	context->setPrimitiveTopology(TOPOLOGY::TRIANGLELIST);

	context->setRenderTargets({ outputTexture.getRTVMipHandle(0) }, {});

	context->setVSConstantBuffer(*pvt.matricesBuffer);

	context->setPSConstants({ inputTexture.getAllSRVIndex() }, 0);

	context->draw(36, 6, 0, 0);
}

void Gear::Core::GlobalEffect::LatLongMapToCubeMapEffect::Internal::initialize(ResourceManager* const resManager)
{
	pvt.equirectangularVS = D3D12Core::Shader::create(g_EquirectangularVSBytes, sizeof(g_EquirectangularVSBytes));

	pvt.equirectangularPS = D3D12Core::Shader::create(g_EquirectangularPSBytes, sizeof(g_EquirectangularPSBytes));

	{
		auto getBuilder = [] {
			return PipelineStateBuilder()
				.setVS(*pvt.equirectangularVS)
				.setPS(*pvt.equirectangularPS)
				.setRasterizerState(PipelineStateHelper::rasterCullNone)
				.setBlendState(PipelineStateHelper::blendReplace)
				.setDepthStencilState(PipelineStateHelper::depthCompareNone)
				.setPrimitiveTopologyType(TOPOLOGY::TYPE::TRIANGLE);
			};

		pvt.equirectangularR8State = getBuilder().setRTVFormats({ FMT::RGBA8UN }).build();

		pvt.equirectangularR16State = getBuilder().setRTVFormats({ FMT::RGBA16F }).build();

		pvt.equirectangularR32State = getBuilder().setRTVFormats({ FMT::RGBA32F }).build();

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

		pvt.matricesBuffer = resManager->createImmutableCBuffer(sizeof(Matrices), &matrices, false);

		pvt.matricesBuffer->getBuffer()->setName(L"LatLongMap To Cubemap Matrices");
	}

	LOGSUCCESS(L"create", LogColor::brightMagenta, L"LatLongMapToCubeMapEffect", LogColor::defaultColor, L"succeeded");
}

void Gear::Core::GlobalEffect::LatLongMapToCubeMapEffect::Internal::release()
{
	pvt.equirectangularVS.reset();

	pvt.equirectangularPS.reset();

	pvt.equirectangularR8State.reset();

	pvt.equirectangularR16State.reset();

	pvt.equirectangularR32State.reset();

	pvt.matricesBuffer.reset();
}

