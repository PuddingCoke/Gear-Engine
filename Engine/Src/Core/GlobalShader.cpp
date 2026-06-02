#include<Gear/Core/GlobalShader.h>

#include<Gear/Core/Internal/GlobalShaderInternal.h>

#include<Gear/CompiledShaders/FullScreenVS.h>

#include<Gear/CompiledShaders/FullScreenPS.h>

#include<Gear/CompiledShaders/TextureCubeVS.h>

namespace Gear::Core::GlobalShader::Internal
{
	struct GlobalShaderImpl
	{
		GlobalShaderImpl();

		UniquePtr<Gear::Core::D3D12Core::Shader> fullScreenVS;

		UniquePtr<Gear::Core::D3D12Core::Shader> fullScreenPS;

		UniquePtr<Gear::Core::D3D12Core::Shader> textureCubeVS;

	};

	GlobalShaderImpl::GlobalShaderImpl()
	{
		fullScreenVS = D3D12Core::Shader::create(g_FullScreenVSBytes, sizeof(g_FullScreenVSBytes));

		fullScreenPS = D3D12Core::Shader::create(g_FullScreenPSBytes, sizeof(g_FullScreenPSBytes));

		textureCubeVS = D3D12Core::Shader::create(g_TextureCubeVSBytes, sizeof(g_TextureCubeVSBytes));
	}
}

namespace
{
	UniquePtr<Gear::Core::GlobalShader::Internal::GlobalShaderImpl> impl;
}

const Gear::Core::D3D12Core::Shader& Gear::Core::GlobalShader::getFullScreenVS()
{
	return *impl->fullScreenVS;
}

const Gear::Core::D3D12Core::Shader& Gear::Core::GlobalShader::getFullScreenPS()
{
	return *impl->fullScreenPS;
}

const Gear::Core::D3D12Core::Shader& Gear::Core::GlobalShader::getTextureCubeVS()
{
	return *impl->textureCubeVS;
}

void Gear::Core::GlobalShader::Internal::initialize()
{
	impl = makeUnique<GlobalShaderImpl>();
}

void Gear::Core::GlobalShader::Internal::release()
{
	impl.reset();
}