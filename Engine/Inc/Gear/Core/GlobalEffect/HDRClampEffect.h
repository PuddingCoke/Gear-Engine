#pragma once

#ifndef _GEAR_CORE_GLOBALEFFECT_HDRCLAMPEFFECT_H_
#define _GEAR_CORE_GLOBALEFFECT_HDRCLAMPEFFECT_H_

#include"GlobalEffectHeader.h"

namespace Gear::Core::GlobalEffect::HDRClampEffect
{
	void process(GraphicsContext& contextRef, Resource::RenderTextureView& inOutTexture);
}

#endif // !_GEAR_CORE_GLOBALCEFFECT_HDRCLAMPEFFECT_H_
