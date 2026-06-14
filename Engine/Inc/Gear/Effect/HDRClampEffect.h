#pragma once

#ifndef _GEAR_EFFECT_HDRCLAMPEFFECT_H_
#define _GEAR_EFFECT_HDRCLAMPEFFECT_H_

#include"EffectBase.h"

namespace Gear::Effect::HDRClampEffect
{
	void process(GraphicsContext& contextRef, RenderTextureView& inOutTexture);
}

#endif // !_GEAR_CORE_GLOBALCEFFECT_HDRCLAMPEFFECT_H_
