#pragma once

#ifndef _GEAR_CORE_RENDERENGINE_H_
#define _GEAR_CORE_RENDERENGINE_H_

#include<Gear/Core/D3D12Core/CommandList.h>

namespace Gear::Core
{
	enum class GPUVendor
	{
		NVIDIA,
		AMD,
		INTEL,
		UNKNOWN
	};

	struct CoInitializeToken { CoInitializeToken() { CHECKERROR(CoInitializeEx(0, COINIT_MULTITHREADED)); } ~CoInitializeToken() { CoUninitialize(); } };

	namespace RenderEngine
	{
		void submitCommandList(D3D12Core::CommandList* const commandList);

		GPUVendor getVendor();

		D3D12Resource::Texture* getRenderTexture();

		ID3D12CommandQueue* getCommandQueue();

		bool getDisplayImGuiSurface();
	}
}

#endif // !_GEAR_CORE_RENDERENGINE_H_