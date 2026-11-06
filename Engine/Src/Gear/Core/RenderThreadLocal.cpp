#include <Gear/Core/RenderThreadLocal.h>

#include <Gear/Core/Internal/RenderThreadLocalInternal.h>

#include <Gear/Core/Internal/LocalDescriptorHeapInternal.h>

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

		//申请每个渲染线程独享的描述符堆
		LocalDescriptorHeap::Internal::InitializeToken localDescriptorHeapToken;

		//存储这一帧拷贝过的资源
		std::vector<Resource::ResourceBase*> copiedResources;
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

		void flushCopiedResources()
		{
			if (impl->copiedResources.size())
			{
				for (Resource::ResourceBase* const resource : impl->copiedResources)
				{
					resource->resetCopyState();
				}

				impl->copiedResources.clear();
			}
		}
	}

	void pushToCopiedResources(Resource::ResourceBase* const resource)
	{
		impl->copiedResources.push_back(resource);
	}
}
