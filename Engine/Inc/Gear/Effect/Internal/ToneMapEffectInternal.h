#pragma once

#ifndef _GEAR_EFFECT_TONEMAPEFFECT_INTERNAL_H_
#define _GEAR_EFFECT_TONEMAPEFFECT_INTERNAL_H_

#include"../EffectBase.h"

namespace Gear::Effect::ToneMapEffect::Internal
{

	void initialize();

	void release();

	struct InitializeToken { InitializeToken() { initialize(); } ~InitializeToken() { release(); } };

}

#endif // !_GEAR_EFFECT_TONEMAPEFFECT_INTERNAL_H_
