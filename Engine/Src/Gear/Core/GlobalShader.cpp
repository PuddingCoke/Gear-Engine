#include<Gear/Core/GlobalShader.h>

#include<Gear/Core/Internal/GlobalShaderInternal.h>

#include<Gear/CompiledShaders/FullScreenVS.h>

#include<Gear/CompiledShaders/FullScreenPS.h>

#include<Gear/CompiledShaders/TextureCubeVS.h>

namespace Gear::Core::GlobalShader
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

	namespace Internal
	{
		void initialize()
		{
			impl = makeUnique<GlobalShaderImpl>();
		}

		void release()
		{
			impl.reset();
		}
	}

	const D3D12Core::Shader* getFullScreenVS()
	{
		return impl->fullScreenVS.get();
	}

	const D3D12Core::Shader* getFullScreenPS()
	{
		return impl->fullScreenPS.get();
	}

	const D3D12Core::Shader* getTextureCubeVS()
	{
		return impl->textureCubeVS.get();
	}
}
