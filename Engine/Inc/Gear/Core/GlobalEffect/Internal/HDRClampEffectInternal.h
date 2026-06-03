#pragma once

#ifndef _GEAR_CORE_GLOBALEFFECT_HDRCLAMPEFFECT_INTERNAL_H_
#define _GEAR_CORE_GLOBALEFFECT_HDRCLAMPEFFECT_INTERNAL_H_

namespace Gear::Core::GlobalEffect::HDRClampEffect::Internal
{
	void initialize();

	void release();

	struct InitializeToken { InitializeToken() { initialize(); } ~InitializeToken() { release(); } };

}

#endif // !_GEAR_CORE_GLOBALEFFECT_HDRCLAMPEFFECT_INTERNAL_H_
