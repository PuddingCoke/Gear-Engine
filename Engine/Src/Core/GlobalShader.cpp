#include<Gear/Core/GlobalShader.h>

#include<Gear/Core/Internal/GlobalShaderInternal.h>

#include<Shaders/CompiledShaders/FullScreenVS.h>

#include<Shaders/CompiledShaders/FullScreenPS.h>

#include<Shaders/CompiledShaders/TextureCubeVS.h>

namespace Gear::Core::GlobalShader
{
	namespace Internal
	{
		struct GlobalShaderImpl
		{
			GlobalShaderImpl();

			D3D12Core::ShaderPtr fullScreenVS;

			D3D12Core::ShaderPtr fullScreenPS;

			D3D12Core::ShaderPtr textureCubeVS;

		};

		GlobalShaderImpl::GlobalShaderImpl()
		{
			fullScreenVS = D3D12Core::Shader::create(g_FullScreenVSBytes, sizeof(g_FullScreenVSBytes));

			fullScreenPS = D3D12Core::Shader::create(g_FullScreenPSBytes, sizeof(g_FullScreenPSBytes));

			textureCubeVS = D3D12Core::Shader::create(g_TextureCubeVSBytes, sizeof(g_TextureCubeVSBytes));
		}

		UniquePtr<GlobalShaderImpl> impl;

		void initialize()
		{
			impl = makeUnique<GlobalShaderImpl>();
		}

		void release()
		{
			impl.reset();
		}
	}

	const D3D12Core::Shader& getFullScreenVS()
	{
		return *Internal::impl->fullScreenVS;
	}

	const D3D12Core::Shader& getFullScreenPS()
	{
		return *Internal::impl->fullScreenPS;
	}

	const D3D12Core::Shader& getTextureCubeVS()
	{
		return *Internal::impl->textureCubeVS;
	}
}