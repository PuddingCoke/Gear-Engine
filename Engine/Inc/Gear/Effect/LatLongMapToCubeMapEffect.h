#pragma once

#ifndef _GEAR_EFFECT_LATLONGMAPTOCUBEMAPEFFECT_H_
#define _GEAR_EFFECT_LATLONGMAPTOCUBEMAPEFFECT_H_

#include"EffectBase.h"

namespace Gear::Effect::LatLongMapToCubeMapEffect
{
	void process(GraphicsContext& contextRef, RenderTextureView& inputTexture, RenderTextureView& outputTexture);
}

#endif // !_GEAR_EFFECT_LATLONGMAPTOCUBEMAPEFFECT_H_
