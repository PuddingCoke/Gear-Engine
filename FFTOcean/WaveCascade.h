#pragma once

#include<Gear/DevEssential.h>

class WaveCascade
{
public:

	WaveCascade(const UINT size, GraphicsContext* const context) :
		size(size), context(context)
	{
		waveDataTexture = createTexture(size, FMT::RGBA32F);

		waveSpectrumTexture = createTexture(size, FMT::RGBA32F);

		DxDz = createTexture(size, FMT::RG32F);

		DyDxz = createTexture(size, FMT::RG32F);

		DyxDyz = createTexture(size, FMT::RG32F);

		DxxDzz = createTexture(size, FMT::RG32F);

		displacementTexture = createTexture(size, FMT::RGBA32F);

		derivativeTexture = createTexture(size, FMT::RGBA32F);

		jacobianTexture = createTexture(size, FMT::R32F);

		displacementTexture->getTexture()->setName(L"displacementTexture");

		derivativeTexture->getTexture()->setName(L"derivativeTexture");

		jacobianTexture->getTexture()->setName(L"jacobianTexture");
	}

	~WaveCascade()
	{
	}

	void ifftPermutation(TextureRenderView& inputTexture)
	{
		context->setPipelineState(*ifftState);

		SETCONSTS({
		context->setCSConstants({
			tempTexture->getUAVMipIndex(0),
			inputTexture.getAllSRVIndex() }, co);
			});

		context->dispatch(size, 1, 1);

		context->uavBarrier({
			tempTexture->getTexture() });

		SETCONSTS({
		context->setCSConstants({
			inputTexture.getUAVMipIndex(0),
			tempTexture->getAllSRVIndex() }, co);
			});

		context->dispatch(size, 1, 1);

		context->uavBarrier({
			inputTexture.getTexture() });

		context->setPipelineState(*permutationState);

		SETCONSTS({
		context->setCSConstants({
			inputTexture.getUAVMipIndex(0) }, co);
			});

		context->dispatch(size / 8, size / 8, 1);

		context->uavBarrier({
			inputTexture.getTexture() });
	}

	void calculateInitialSpectrum(ImmutableCBuffer& spectrumParamBuffer)
	{
		context->setPipelineState(*spectrumState);

		SETCONSTS({
		context->setCSConstants({
			tildeh0Texture->getUAVMipIndex(0),
			waveDataTexture->getUAVMipIndex(0),
			randomGaussTexture->getAllSRVIndex() }, co);
			});

		context->setCSConstantBuffer(spectrumParamBuffer);

		context->dispatch(size / 8, size / 8, 1);

		context->uavBarrier({
			tildeh0Texture->getTexture(),
			waveDataTexture->getTexture() });

		context->setPipelineState(*conjugateState);

		SETCONSTS({
		context->setCSConstants({
			waveSpectrumTexture->getUAVMipIndex(0),
			tildeh0Texture->getAllSRVIndex() }, co);
			});

		context->dispatch(size / 8, size / 8, 1);

		context->uavBarrier({
			waveSpectrumTexture->getTexture() });
	}

	void calculateTimeDependentSpectrum()
	{
		context->setPipelineState(*displacementSpectrumState);

		SETCONSTS({
		context->setCSConstants({
			DxDz->getUAVMipIndex(0),
			DyDxz->getUAVMipIndex(0),
			DyxDyz->getUAVMipIndex(0),
			DxxDzz->getUAVMipIndex(0),
			waveDataTexture->getAllSRVIndex(),
			waveSpectrumTexture->getAllSRVIndex() }, co);
			});

		context->dispatch(size / 8, size / 8, 1);

		context->uavBarrier({
			DxDz->getTexture(),
			DyDxz->getTexture(),
			DyxDyz->getTexture(),
			DxxDzz->getTexture() });
	}

	void calculateDisplacementAndDerivative()
	{
		ifftPermutation(*DxDz);

		ifftPermutation(*DyDxz);

		ifftPermutation(*DyxDyz);

		ifftPermutation(*DxxDzz);

		context->setPipelineState(*waveMergeState);

		SETCONSTS({
		context->setCSConstants({
			displacementTexture->getUAVMipIndex(0),
			derivativeTexture->getUAVMipIndex(0),
			jacobianTexture->getUAVMipIndex(0),
			DxDz->getAllSRVIndex(),
			DyDxz->getAllSRVIndex(),
			DyxDyz->getAllSRVIndex(),
			DxxDzz->getAllSRVIndex() }, co);
			});

		context->dispatch(size / 8, size / 8, 1);

		context->uavBarrier({
			displacementTexture->getTexture(),
			derivativeTexture->getTexture(),
			jacobianTexture->getTexture() });
	}

	static PipelineState* spectrumState;

	static PipelineState* conjugateState;

	static PipelineState* displacementSpectrumState;

	static PipelineState* ifftState;

	static PipelineState* permutationState;

	static PipelineState* waveMergeState;

	static TextureRenderView* tempTexture;

	static TextureRenderView* randomGaussTexture;

	//(tildeh0(k))
	//x y
	static TextureRenderView* tildeh0Texture;

	//Dx Dy Dz
	UniquePtr<TextureRenderView> displacementTexture;

	//dDy/dx dDy/dz dDx/dx dDz/dz
	UniquePtr<TextureRenderView> derivativeTexture;

	//J
	UniquePtr<TextureRenderView> jacobianTexture;

private:

	static UniquePtr<TextureRenderView> createTexture(const UINT& resolution, const DXGI_FORMAT& format)
	{
		return ResourceManager::createTextureRenderView(resolution, resolution, format, 1, 1, false, true, format, format, FMT::UNKNOWN);
	}

	const UINT size;

	GraphicsContext* const context;

	//k.x 1.0/length(K) k.z angularSpeed
	//x y z w
	UniquePtr<TextureRenderView> waveDataTexture;

	//(tildeh0(k), conj(tildeh0(-k)))
	//x y z w
	UniquePtr<TextureRenderView> waveSpectrumTexture;

	//Dx Dz
	UniquePtr<TextureRenderView> DxDz;

	//Dy dDx/dz
	UniquePtr<TextureRenderView> DyDxz;

	//dDy/dx dDy/dz
	UniquePtr<TextureRenderView> DyxDyz;

	//dDx/dx dDz/dz
	UniquePtr<TextureRenderView> DxxDzz;

};

PipelineState* WaveCascade::spectrumState = nullptr;

PipelineState* WaveCascade::conjugateState = nullptr;

PipelineState* WaveCascade::displacementSpectrumState = nullptr;

PipelineState* WaveCascade::ifftState = nullptr;

PipelineState* WaveCascade::permutationState = nullptr;

PipelineState* WaveCascade::waveMergeState = nullptr;

TextureRenderView* WaveCascade::tempTexture = nullptr;

TextureRenderView* WaveCascade::randomGaussTexture = nullptr;

TextureRenderView* WaveCascade::tildeh0Texture = nullptr;