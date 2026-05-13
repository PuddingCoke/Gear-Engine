#pragma once

#ifndef _GEAR_CORE_MAINCAMERA_INTERNAL_H_
#define _GEAR_CORE_MAINCAMERA_INTERNAL_H_

#include<Gear/Utils/Math.h>

namespace Gear
{
	namespace Core
	{
		namespace MainCamera
		{
			namespace Internal
			{
				struct Matrices
				{
					DirectX::XMMATRIX proj;
					DirectX::XMMATRIX view;
					DirectX::XMVECTOR eyePos;
					DirectX::XMMATRIX prevViewProj;
					DirectX::XMMATRIX viewProj;
					DirectX::XMMATRIX normalMatrix;
				};

				Matrices getMatrices();
			}
		}
	}
}

#endif // !_GEAR_CORE_MAINCAMERA_INTERNAL_H_
