#pragma once

#ifndef _GEAR_EFFECT_GAMMACORRECTEFFECT_INTERNAL_H
#define _GEAR_EFFECT_GAMMACORRECTEFFECT_INTERNAL_H

#include"../EffectBase.h"

namespace Gear::Effect::GammaCorrectEffect::Internal
{

	void initialize();

	void release();

	struct InitializeToken { InitializeToken() { initialize(); } ~InitializeToken() { release(); } };

}

#endif // !_GEAR_EFFECT_GAMMACORRECTEFFECT_INTERNAL_H
