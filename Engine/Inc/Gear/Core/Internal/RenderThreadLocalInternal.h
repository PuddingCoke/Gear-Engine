#pragma once

#ifndef _GEAR_CORE_RENDERTHREADLOCAL_INTERNAL_H_
#define _GEAR_CORE_RENDERTHREADLOCAL_INTERNAL_H_

namespace Gear::Core::RenderThreadLocal::Internal
{
	void initialize();

	void release();

	void flushCopiedResources();

	struct InitializeToken { InitializeToken() { initialize(); } ~InitializeToken() { release(); } };
}

#endif // !_GEAR_CORE_RENDERTHREADLOCAL_INTERNAL_H_