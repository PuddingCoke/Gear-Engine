#pragma once

#ifndef _GEAR_CORE_DYNAMICCBUFFERMANAGER_INTERNAL_H_
#define _GEAR_CORE_DYNAMICCBUFFERMANAGER_INTERNAL_H_

#include<Gear/Core/D3D12Core/CommandList.h>

namespace Gear::Core::DynamicCBufferManager::Internal
{
	void initialize();

	void release();

	struct InitializeToken { InitializeToken() { initialize(); } ~InitializeToken() { release(); } };

	void recordCommands(D3D12Core::CommandList* const commandList);
}

#endif // !_GEAR_CORE_DYNAMICCBUFFERMANAGER_INTERNAL_H_
