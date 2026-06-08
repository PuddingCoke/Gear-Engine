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

		float clearVal[] = { 9999.f,9999.f,9999.f,9999.f };

		context->clearUnorderedAccess(jacobianTexture->getClearUAVMipDesc(0), clearVal);
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

		context->dispatch(size, 1, 1);

		SETCONSTS({
		context->setCSConstants({
			inputTexture.getUAVMipIndex(0),
			tempTexture->getAllSRVIndex() }, co);
			});

		context->dispatch(size, 1, 1);

		context->setPipelineState(*permutationState);

		SETCONSTS({
		context->setCSConstants({
			inputTexture.getUAVMipIndex(0) }, co);
			});

		context->dispatch(size / 8, size / 8, 1);
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

		context->setPipelineState(*conjugateState);

		SETCONSTS({
		context->setCSConstants({
			waveSpectrumTexture->getUAVMipIndex(0),
			tildeh0Texture->getAllSRVIndex() }, co);
			});

		context->dispatch(size / 8, size / 8, 1);
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
	UniquePtr<RenderTextureView> displacementTexture;

	//dDy/dx dDy/dz dDx/dx dDz/dz
	UniquePtr<RenderTextureView> derivativeTexture;

	//J
	UniquePtr<RenderTextureView> jacobianTexture;

private:

	static UniquePtr<RenderTextureView> createTexture(const UINT& resolution, const DXGI_FORMAT& format)
	{
		return ResourceManager::createComputeTexture(resolution, resolution, format, 1, 1, false, true);
	}

	const UINT size;

	GraphicsContext* const context;

	//k.x 1.0/length(K) k.z angularSpeed
	//x y z w
	UniquePtr<RenderTextureView> waveDataTexture;

	//(tildeh0(k), conj(tildeh0(-k)))
	//x y z w
	UniquePtr<RenderTextureView> waveSpectrumTexture;

	//Dx Dz
	UniquePtr<RenderTextureView> DxDz;

	//Dy dDx/dz
	UniquePtr<RenderTextureView> DyDxz;

	//dDy/dx dDy/dz
	UniquePtr<RenderTextureView> DyxDyz;

	//dDx/dx dDz/dz
	UniquePtr<RenderTextureView> DxxDzz;

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