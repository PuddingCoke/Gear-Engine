#pragma once

#ifndef _GEAR_CORE_RENDERENGINE_H_
#define _GEAR_CORE_RENDERENGINE_H_

struct ImFont;

struct ID3D12CommandQueue;

namespace Gear::Core
{
	namespace D3D12Core
	{
		class CommandList;
	}

	namespace D3D12Resource
	{
		class Texture;
	}

	enum class AdapterVendor
	{
		NVIDIA,
		AMD,
		INTEL,
		UNKNOWN
	};

	namespace RenderEngine
	{
		void submitCommandList(D3D12Core::CommandList* const commandList);

		AdapterVendor getVendor();

		D3D12Resource::Texture* getRenderTexture();

		ID3D12CommandQueue* getCommandQueue();

		bool getDisplayImGuiSurface();

		void toggleImGuiSurface();

		void toggleEngineImGuiSurface();

		ImFont* getMediumFont();

		ImFont* getLargeFont();

	}
}

#endif // !_GEAR_CORE_RENDERENGINE_H_