#pragma once

#ifndef _GEAR_CORE_GLOBALEFFECT_GAMMACORRECTEFFECT_INTERNAL_H
#define _GEAR_CORE_GLOBALEFFECT_GAMMACORRECTEFFECT_INTERNAL_H

namespace Gear::Core::GlobalEffect::GammaCorrectEffect::Internal
{
	void initialize();

	void release();

	struct InitializeToken { InitializeToken() { initialize(); } ~InitializeToken() { release(); } };

}

#endif // !_GEAR_CORE_GLOBALEFFECT_GAMMACORRECTEFFECT_INTERNAL_H
