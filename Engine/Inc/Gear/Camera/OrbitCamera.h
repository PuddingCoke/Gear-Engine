#pragma once

#ifndef _GEAR_CAMERA_ORBITCAMERA_H_
#define _GEAR_CAMERA_ORBITCAMERA_H_

#include <Gear/Utils/Math.h>

namespace Gear
{
	namespace Camera
	{
		class OrbitCamera
		{
		public:

			OrbitCamera() = delete;

			OrbitCamera(const OrbitCamera&) = delete;

			void operator=(const OrbitCamera&) = delete;

			OrbitCamera(const DirectX::XMVECTOR& eye,
			            const DirectX::XMVECTOR& up,
			            const float zoomSpeed,
			            const bool smoothZoom = true);

			~OrbitCamera();

			void applyInput(const float dt);

			void rotateX(const float dTheta);

			void rotateY(const float dTheta);

		private:

			float currentRadius;

			float targetRadius;

			const float zoomSpeed;

			uint64_t moveEventID;

			uint64_t scrollEventID;

			DirectX::XMVECTOR eye;

			const DirectX::XMVECTOR up;

			const bool smoothZoom;
		};
	}
}

#endif // !_GEAR_CAMERA_ORBITCAMERA_H_
