#pragma once

#ifndef _GEAR_CORE_GLOBALEFFECT_HDRCLAMPEFFECT_H_
#define _GEAR_CORE_GLOBALEFFECT_HDRCLAMPEFFECT_H_

#include"GlobalEffectHeader.h"

namespace Gear::Core::GlobalEffect::HDRClampEffect
{
	void process(GraphicsContext* const context, Resource::TextureRenderView& inOutTexture);
}

#endif // !_GEAR_CORE_GLOBALCEFFECT_HDRCLAMPEFFECT_H_
