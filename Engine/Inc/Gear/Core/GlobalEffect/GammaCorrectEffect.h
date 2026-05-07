#pragma once

#ifndef _GEAR_CORE_GLOBALEFFECT_GAMMACORRECTEFFECT_H_
#define _GEAR_CORE_GLOBALEFFECT_GAMMACORRECTEFFECT_H_

#include"GlobalEffectHeader.h"

namespace Gear
{
	namespace Core
	{
		namespace GlobalEffect
		{
			namespace GammaCorrectEffect
			{
				Resource::TextureRenderView* process(GraphicsContext* const context, Resource::TextureRenderView* const inputTexture);
			}
		}
	}
}

#endif // !_GEAR_CORE_GLOBALEFFECT_GAMMACORRECTEFFECT_H_