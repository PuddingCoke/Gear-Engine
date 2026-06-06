#pragma once

#ifndef _GEAR_CORE_GLOBALEFFECT_TONEMAPEFFECT_H_
#define _GEAR_CORE_GLOBALEFFECT_TONEMAPEFFECT_H_

#include"GlobalEffectHeader.h"

namespace Gear::Core::GlobalEffect::ToneMapEffect
{
	Resource::RenderTextureView* process(GraphicsContext* const context, Resource::RenderTextureView& inputTexture);
}

#endif // !_GEAR_CORE_GLOBALEFFECT_TONEMAPEFFECT_H_

