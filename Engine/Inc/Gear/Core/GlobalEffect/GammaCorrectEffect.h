#pragma once

#ifndef _GEAR_CORE_GLOBALEFFECT_GAMMACORRECTEFFECT_H_
#define _GEAR_CORE_GLOBALEFFECT_GAMMACORRECTEFFECT_H_

#include"GlobalEffectHeader.h"

namespace Gear::Core::GlobalEffect::GammaCorrectEffect
{
	Resource::TextureRenderView* process(GraphicsContext* const context, Resource::TextureRenderView& inputTexture);
}

#endif // !_GEAR_CORE_GLOBALEFFECT_GAMMACORRECTEFFECT_H_