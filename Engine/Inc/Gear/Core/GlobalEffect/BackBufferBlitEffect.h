#pragma once

#ifndef _GEAR_CORE_GLOBALEFFECT_BACKBUFFERBLITEFFECT_H_
#define _GEAR_CORE_GLOBALEFFECT_BACKBUFFERBLITEFFECT_H_

#include"GlobalEffectHeader.h"

namespace Gear::Core::GlobalEffect::BackBufferBlitEffect
{
	void process(GraphicsContext* const context, Resource::RenderTextureView& inputTexture);
}

#endif // !_GEAR_CORE_GLOBALEFFECT_BACKBUFFERBLITEFFECT_H_
