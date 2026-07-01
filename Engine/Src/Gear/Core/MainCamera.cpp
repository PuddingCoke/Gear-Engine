#include<Gear/Core/MainCamera.h>

namespace Gear::Core::MainCamera
{
	namespace Internal
	{
		struct MainCameraImpl
		{

			DirectX::XMMATRIX projMatrix = {};

			DirectX::XMMATRIX viewMatrix = {};

			DirectX::XMVECTOR eyePos = {};

		}impl;
	}

	DirectX::XMMATRIX getProj()
	{
		return Internal::impl.projMatrix;
	}

	DirectX::XMMATRIX getView()
	{
		return Internal::impl.viewMatrix;
	}

	DirectX::XMVECTOR getEyePos()
	{
		return Internal::impl.eyePos;
	}

	void setProj(const float fov, const float aspectRatio, const float zNear, const float zFar)
	{
		setProj(DirectX::XMMatrixPerspectiveFovLH(fov, aspectRatio, zNear, zFar));
	}

	void setView(const DirectX::XMVECTOR& eyePos, const DirectX::XMVECTOR& focus, const DirectX::XMVECTOR& up)
	{
		Internal::impl.eyePos = eyePos;

		setView(DirectX::XMMatrixLookAtLH(eyePos, focus, up));
	}

	void setProj(const DirectX::XMMATRIX& proj)
	{
		Internal::impl.projMatrix = proj;
	}

	void setView(const DirectX::XMMATRIX& view)
	{
		Internal::impl.viewMatrix = view;
	}
}