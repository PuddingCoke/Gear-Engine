#pragma once

#ifndef _GEAR_EFFECT_TONEMAPEFFECT_H_
#define _GEAR_EFFECT_TONEMAPEFFECT_H_

#include"EffectBase.h"

namespace Gear::Effect::ToneMapEffect
{
	RenderTextureView* process(GraphicsContext& contextRef, RenderTextureView& inputTexture);
}

#endif // !_GEAR_EFFECT_TONEMAPEFFECT_H_

