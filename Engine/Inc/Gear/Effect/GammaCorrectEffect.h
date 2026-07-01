#pragma once

#ifndef _GEAR_EFFECT_GAMMACORRECTEFFECT_H_
#define _GEAR_EFFECT_GAMMACORRECTEFFECT_H_

#include"EffectBase.h"

namespace Gear::Effect::GammaCorrectEffect
{
	RenderTextureView* process(GraphicsContext& contextRef, RenderTextureView& inputTexture);
}

#endif // !_GEAR_EFFECT_GAMMACORRECTEFFECT_H_