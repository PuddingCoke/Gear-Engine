#include <Gear/Core/MainCamera.h>

namespace Gear::Core::MainCamera
{
	struct MainCameraImpl
	{
		DirectX::XMMATRIX projMatrix = {};

		DirectX::XMMATRIX viewMatrix = {};

		DirectX::XMVECTOR cameraPos = {};

		float fovAngleY = 0.f;

		float zNear = 0.f;

		float zFar = 0.f;

	} impl;

	DirectX::XMMATRIX getProj()
	{
		return impl.projMatrix;
	}

	DirectX::XMMATRIX getView()
	{
		return impl.viewMatrix;
	}

	DirectX::XMVECTOR getEyePos()
	{
		return impl.cameraPos;
	}

	float getFovAngleY()
	{
		return impl.fovAngleY;
	}

	float getZNear()
	{
		return impl.zNear;
	}

	float getZFar()
	{
		return impl.zFar;
	}

	void setProj(const float fov, const float aspectRatio, const float zNear, const float zFar)
	{
		impl.fovAngleY = fov;

		impl.zNear = zNear;

		impl.zFar = zFar;

		setProj(DirectX::XMMatrixPerspectiveFovLH(fov, aspectRatio, zNear, zFar));
	}

	void setView(const DirectX::XMVECTOR& cameraPos,
	             const DirectX::XMVECTOR& focus,
	             const DirectX::XMVECTOR& up)
	{
		impl.cameraPos = cameraPos;

		setView(DirectX::XMMatrixLookAtLH(cameraPos, focus, up));
	}

	void setProj(const DirectX::XMMATRIX& proj)
	{
		impl.projMatrix = proj;
	}

	void setView(const DirectX::XMMATRIX& view)
	{
		impl.viewMatrix = view;
	}
}
