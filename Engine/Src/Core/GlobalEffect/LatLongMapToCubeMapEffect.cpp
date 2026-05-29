#include<Gear/Core/GlobalEffect/LatLongMapToCubeMapEffect.h>

#include<Gear/Core/GlobalEffect/Internal/LatLongMapToCubeMapEffectInternal.h>

#include<Gear/Utils/Math.h>

#include<Gear/CompiledShaders/EquirectangularVS.h>

#include<Gear/CompiledShaders/EquirectangularPS.h>

namespace
{
	struct LatLongMapToCubeMapEffectPrivate
	{

		Gear::Core::D3D12Core::Shader* equirectangularVS;

		Gear::Core::D3D12Core::Shader* equirectangularPS;

		Gear::Core::D3D12Core::PipelineState* equirectangularR8State;

		Gear::Core::D3D12Core::PipelineState* equirectangularR16State;

		Gear::Core::D3D12Core::PipelineState* equirectangularR32State;

		Gear::Core::Resource::ImmutableCBuffer* matricesBuffer;

	}pvt;
}

void Gear::Core::GlobalEffect::LatLongMapToCubeMapEffect::process(GraphicsContext* const context, Resource::TextureRenderView* const inputTexture, Resource::TextureRenderView* const outputTexture)
{
	switch (outputTexture->getTexture()->getFormat())
	{
	case FMT::RGBA8UN:
		context->setPipelineState(pvt.equirectangularR8State);
		break;
	case FMT::RGBA16F:
		context->setPipelineState(pvt.equirectangularR16State);
		break;
	case FMT::RGBA32F:
		context->setPipelineState(pvt.equirectangularR32State);
		break;
	}

	context->setViewportSimple(outputTexture->getTexture()->getWidth(), outputTexture->getTexture()->getHeight());

	context->setPrimitiveTopology(TOPOLOGY::TRIANGLELIST);

	context->setRenderTargets({ outputTexture->getRTVMipHandle(0) }, {});

	context->setVSConstantBuffer(pvt.matricesBuffer);

	context->setPSConstants({ inputTexture->getAllSRVIndex() }, 0);

	context->draw(36, 6, 0, 0);
}

void Gear::Core::GlobalEffect::LatLongMapToCubeMapEffect::Internal::initialize(ResourceManager* const resManager)
{
	pvt.equirectangularVS = new D3D12Core::Shader(g_EquirectangularVSBytes, sizeof(g_EquirectangularVSBytes));

	pvt.equirectangularPS = new D3D12Core::Shader(g_EquirectangularPSBytes, sizeof(g_EquirectangularPSBytes));

	{
		auto getBuilder = [] {
			return PipelineStateBuilder()
				.setVS(pvt.equirectangularVS)
				.setPS(pvt.equirectangularPS)
				.setRasterizerState(PipelineStateHelper::rasterCullNone)
				.setBlendState(CD3DX12_BLEND_DESC(D3D12_DEFAULT))
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
	if (pvt.equirectangularVS)
	{
		delete pvt.equirectangularVS;
	}

	if (pvt.equirectangularPS)
	{
		delete pvt.equirectangularPS;
	}

	if (pvt.equirectangularR8State)
	{
		delete pvt.equirectangularR8State;
	}

	if (pvt.equirectangularR16State)
	{
		delete pvt.equirectangularR16State;
	}

	if (pvt.equirectangularR32State)
	{
		delete pvt.equirectangularR32State;
	}

	if (pvt.matricesBuffer)
	{
		delete pvt.matricesBuffer;
	}
}

