#pragma once

#ifndef _GEAR_CORE_MAINCAMERA_H_
#define _GEAR_CORE_MAINCAMERA_H_

#include<Gear/Utils/Math.h>

namespace Gear::Core::MainCamera
{
	DirectX::XMMATRIX getProj();

	DirectX::XMMATRIX getView();

	DirectX::XMVECTOR getEyePos();

	void setProj(const float fov, const float aspectRatio, const float zNear, const float zFar);

	void setView(const DirectX::XMVECTOR& eyePos, const DirectX::XMVECTOR& focus, const DirectX::XMVECTOR& up);

	void setProj(const DirectX::XMMATRIX& proj);

	void setView(const DirectX::XMMATRIX& view);

	constexpr float epsilon = 0.01f;
}

#endif // !_GEAR_CORE_MAINCAMERA_H_