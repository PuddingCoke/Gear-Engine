#pragma once

#include<Gear/DevEssential.h>

class WaveCascade
{
public:

	WaveCascade(const uint32_t size, GraphicsContext* const context) :
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

		float clearVal[] = { 9999.f,9999.f,9999.f,9999.f };

		context->clearUnorderedAccess(jacobianTexture->getUAVMipClearDesc(0), clearVal);
	}

	~WaveCascade()
	{
	}

	void ifftPermutation(RenderTextureView& inputTexture)
	{
		context->setPipelineState(*ifftState);

		SETCONSTS({
		context->setCSConstants({
			tempTexture->getUAVMipIndex(0),
			inputTexture.getAllSRVIndex() }, co);
			});

		context->dispatchDim(size* size, 1, 1);

		SETCONSTS({
		context->setCSConstants({
			inputTexture.getUAVMipIndex(0),
			tempTexture->getAllSRVIndex() }, co);
			});

		context->dispatchDim(size* size, 1, 1);

		context->setPipelineState(*permutationState);

		SETCONSTS({
		context->setCSConstants({
			inputTexture.getUAVMipIndex(0) }, co);
			});

		context->dispatchDim(size, size, 1);
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

		context->dispatchDim(size, size, 1);

		context->setPipelineState(*conjugateState);

		SETCONSTS({
		context->setCSConstants({
			waveSpectrumTexture->getUAVMipIndex(0),
			tildeh0Texture->getAllSRVIndex() }, co);
			});

		context->dispatchDim(size, size, 1);
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

		context->dispatchDim(size, size, 1);
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

		context->dispatchDim(size, size, 1);
	}

	static PipelineState* spectrumState;

	static PipelineState* conjugateState;

	static PipelineState* displacementSpectrumState;

	static PipelineState* ifftState;

	static PipelineState* permutationState;

	static PipelineState* waveMergeState;

	static RenderTextureView* tempTexture;

	static RenderTextureView* randomGaussTexture;

	//(tildeh0(k))
	//x y
	static RenderTextureView* tildeh0Texture;

	//Dx Dy Dz
	RenderTextureViewPtr displacementTexture;

	//dDy/dx dDy/dz dDx/dx dDz/dz
	RenderTextureViewPtr derivativeTexture;

	//J
	RenderTextureViewPtr jacobianTexture;

private:

	static RenderTextureViewPtr createTexture(const uint32_t& resolution, const DXGI_FORMAT& format)
	{
		return ResourceManager::createComputeTexture(resolution, resolution, format, 1, 1, false, true);
	}

	const uint32_t size;

	GraphicsContext* const context;

	//k.x 1.0/length(K) k.z angularSpeed
	//x y z w
	RenderTextureViewPtr waveDataTexture;

	//(tildeh0(k), conj(tildeh0(-k)))
	//x y z w
	RenderTextureViewPtr waveSpectrumTexture;

	//Dx Dz
	RenderTextureViewPtr DxDz;

	//Dy dDx/dz
	RenderTextureViewPtr DyDxz;

	//dDy/dx dDy/dz
	RenderTextureViewPtr DyxDyz;

	//dDx/dx dDz/dz
	RenderTextureViewPtr DxxDzz;

};

PipelineState* WaveCascade::spectrumState = nullptr;

PipelineState* WaveCascade::conjugateState = nullptr;

PipelineState* WaveCascade::displacementSpectrumState = nullptr;

PipelineState* WaveCascade::ifftState = nullptr;

PipelineState* WaveCascade::permutationState = nullptr;

PipelineState* WaveCascade::waveMergeState = nullptr;

RenderTextureView* WaveCascade::tempTexture = nullptr;

RenderTextureView* WaveCascade::randomGaussTexture = nullptr;

RenderTextureView* WaveCascade::tildeh0Texture = nullptr;