#pragma once

#ifndef _GEAR_EFFECT_BACKBUFFERBLITEFFECT_H_
#define _GEAR_EFFECT_BACKBUFFERBLITEFFECT_H_

#include"EffectBase.h"

namespace Gear::Effect::BackBufferBlitEffect
{
	void process(GraphicsContext& contextRef, RenderTextureView& inputTexture);
}

#endif // !_GEAR_EFFECT_BACKBUFFERBLITEFFECT_H_
