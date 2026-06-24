#pragma once

#ifndef _GEAR_CAMERA_FPSCAMERA_H_
#define _GEAR_CAMERA_FPSCAMERA_H_

#include<Gear/Utils/Math.h>

namespace Gear
{
	namespace Camera
	{
		class FPSCamera
		{
		public:

			FPSCamera() = delete;

			FPSCamera(const FPSCamera&) = delete;

			void operator=(const FPSCamera&) = delete;

			FPSCamera(const DirectX::XMVECTOR& eye, const DirectX::XMVECTOR& lookDir, const DirectX::XMVECTOR& up, const float moveSpeed);

			~FPSCamera();

			void applyInput(const float dt);

			DirectX::XMVECTOR getEyePos() const;

			DirectX::XMVECTOR getLookDir() const;

			DirectX::XMVECTOR getUpVector() const;

		private:

			const float moveSpeed;

			DirectX::XMVECTOR eye;

			DirectX::XMVECTOR lookDir;

			DirectX::XMVECTOR up;

			uint64_t moveEventID;

		};
	}
}

#endif // !_GEAR_CAMERA_FPSCAMERA_H_
