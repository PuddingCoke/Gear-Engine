#include <Gear/Core/RenderThreadLocal.h>

#include <Gear/Core/GraphicsDevice.h>

#include <Gear/Core/Internal/RenderThreadLocalInternal.h>

#include <Gear/Core/D3D12Core/Internal/DXCCompilerInternal.h>

namespace Gear::Core::RenderThreadLocal
{
	struct CoInitializeToken
	{
		CoInitializeToken()
		{
			CHECKERROR(CoInitializeEx(0, COINIT_MULTITHREADED));
		}
		~CoInitializeToken()
		{
			CoUninitialize();
		}
	};

	struct RenderThreadLocalImpl
	{
		//初始化COM组件，DirectXTex需要
		CoInitializeToken coInitializeToken;

		//初始化每个渲染线程独享的DXC编译器
		D3D12Core::DXCCompiler::Internal::InitializeToken dxcCompilerToken;
	};

	thread_local UniquePtr<RenderThreadLocalImpl> impl;

	namespace Internal
	{
		void initialize()
		{
			impl = makeUnique<RenderThreadLocalImpl>();
		}

		void release()
		{
			impl.reset();
		}
	}

}
