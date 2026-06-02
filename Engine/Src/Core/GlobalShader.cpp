#include<Gear/Core/GlobalShader.h>

#include<Gear/Core/Internal/GlobalShaderInternal.h>

#include<Gear/CompiledShaders/FullScreenVS.h>

#include<Gear/CompiledShaders/FullScreenPS.h>

#include<Gear/CompiledShaders/TextureCubeVS.h>

namespace
{
	struct GlobalShaderImpl
	{

		UniquePtr<Gear::Core::D3D12Core::Shader> fullScreenVS;

		UniquePtr<Gear::Core::D3D12Core::Shader> fullScreenPS;

		UniquePtr<Gear::Core::D3D12Core::Shader> textureCubeVS;

	}impl;
}

const Gear::Core::D3D12Core::Shader& Gear::Core::GlobalShader::getFullScreenVS()
{
	return *impl.fullScreenVS;
}

const Gear::Core::D3D12Core::Shader& Gear::Core::GlobalShader::getFullScreenPS()
{
	return *impl.fullScreenPS;
}

const Gear::Core::D3D12Core::Shader& Gear::Core::GlobalShader::getTextureCubeVS()
{
	return *impl.textureCubeVS;
}

void Gear::Core::GlobalShader::Internal::initialize()
{
	impl.fullScreenVS = D3D12Core::Shader::create(g_FullScreenVSBytes, sizeof(g_FullScreenVSBytes));

	impl.fullScreenPS = D3D12Core::Shader::create(g_FullScreenPSBytes, sizeof(g_FullScreenPSBytes));

	impl.textureCubeVS = D3D12Core::Shader::create(g_TextureCubeVSBytes, sizeof(g_TextureCubeVSBytes));
}

void Gear::Core::GlobalShader::Internal::release()
{
	impl.fullScreenVS.reset();

	impl.fullScreenPS.reset();

	impl.textureCubeVS.reset();
}