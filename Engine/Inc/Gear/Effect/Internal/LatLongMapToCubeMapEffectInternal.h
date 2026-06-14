#pragma once

#ifndef _GEAR_EFFECT_LATLONGMAPTOCUBEMAPEFFECT_INTERNAL_H_
#define _GEAR_EFFECT_LATLONGMAPTOCUBEMAPEFFECT_INTERNAL_H_

#include"../EffectBase.h"

namespace Gear::Effect::LatLongMapToCubeMapEffect::Internal
{

	void initialize(ResourceManager* const resManager);

	void release();

	struct InitializeToken { InitializeToken(ResourceManager* const resManager) { initialize(resManager); } ~InitializeToken() { release(); } };

}

#endif // !_GEAR_EFFECT_LATLONGMAPTOCUBEMAPEFFECT_INTERNAL_H_
