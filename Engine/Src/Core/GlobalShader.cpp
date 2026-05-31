#include<Gear/Core/GlobalShader.h>

#include<Gear/Core/Internal/GlobalShaderInternal.h>

#include<Gear/CompiledShaders/FullScreenVS.h>

#include<Gear/CompiledShaders/FullScreenPS.h>

#include<Gear/CompiledShaders/TextureCubeVS.h>

namespace
{
	struct GlobalShaderPrivate
	{

		UniquePtr<Gear::Core::D3D12Core::Shader> fullScreenVS;

		UniquePtr<Gear::Core::D3D12Core::Shader> fullScreenPS;

		UniquePtr<Gear::Core::D3D12Core::Shader> textureCubeVS;

	}pvt;
}

const Gear::Core::D3D12Core::Shader& Gear::Core::GlobalShader::getFullScreenVS()
{
	return *pvt.fullScreenVS;
}

const Gear::Core::D3D12Core::Shader& Gear::Core::GlobalShader::getFullScreenPS()
{
	return *pvt.fullScreenPS;
}

const Gear::Core::D3D12Core::Shader& Gear::Core::GlobalShader::getTextureCubeVS()
{
	return *pvt.textureCubeVS;
}

void Gear::Core::GlobalShader::Internal::initialize()
{
	pvt.fullScreenVS = D3D12Core::Shader::create(g_FullScreenVSBytes, sizeof(g_FullScreenVSBytes));

	pvt.fullScreenPS = D3D12Core::Shader::create(g_FullScreenPSBytes, sizeof(g_FullScreenPSBytes));

	pvt.textureCubeVS = D3D12Core::Shader::create(g_TextureCubeVSBytes, sizeof(g_TextureCubeVSBytes));
}

void Gear::Core::GlobalShader::Internal::release()
{
	pvt.fullScreenVS.reset();

	pvt.fullScreenPS.reset();

	pvt.textureCubeVS.reset();
}