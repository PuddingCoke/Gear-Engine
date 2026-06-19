#include<Gear/Core/RenderThreadGlobal.h>

#include<Gear/Core/Internal/RenderThreadGlobalInternal.h>

#include<Gear/Core/Internal/GlobalShaderInternal.h>

#include<Gear/Core/Internal/GlobalDescriptorHeapInternal.h>

#include<Gear/Core/Internal/GlobalRootSignatureInternal.h>

#include<Gear/Core/Internal/DynamicCBufferManagerInternal.h>

namespace Gear::Core::RenderThreadGlobal
{
	namespace Internal
	{
		struct RenderThreadGlobalImpl
		{
			GlobalShader::Internal::InitializeToken globalShader;

			GlobalDescriptorHeap::Internal::InitializeToken globalDescriptorHeap;

			GlobalRootSignature::Internal::InitializeToken globalRootSignature;

			DynamicCBufferManager::Internal::InitializeToken dynamicCBufferManager;
		};

		UniquePtr<RenderThreadGlobalImpl> impl;

		void initialize()
		{
			impl = makeUnique<RenderThreadGlobalImpl>();
		}

		void release()
		{
			impl.reset();
		}
	}
}