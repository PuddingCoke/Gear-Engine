#pragma once

#ifndef _GEAR_CORE_GLOBALEFFECT_BACKBUFFERBLITEFFECT_H_
#define _GEAR_CORE_GLOBALEFFECT_BACKBUFFERBLITEFFECT_H_

#include"GlobalEffectHeader.h"

namespace Gear
{
	namespace Core
	{
		namespace GlobalEffect
		{
			namespace BackBufferBlitEffect
			{
				void process(GraphicsContext* const context, Resource::TextureRenderView& inputTexture);
			}
		}
	}
}

#endif // !_GEAR_CORE_GLOBALEFFECT_BACKBUFFERBLITEFFECT_H_
