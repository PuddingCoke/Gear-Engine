#include<Gear/Core/MainCamera.h>

namespace
{
	struct MainCameraPrivate
	{

		DirectX::XMMATRIX projMatrix = {};

		DirectX::XMMATRIX viewMatrix = {};

		DirectX::XMVECTOR eyePos = {};

	}pvt;
}

DirectX::XMMATRIX Gear::Core::MainCamera::getProj()
{
	return pvt.projMatrix;
}

DirectX::XMMATRIX Gear::Core::MainCamera::getView()
{
	return pvt.viewMatrix;
}

DirectX::XMVECTOR Gear::Core::MainCamera::getEyePos()
{
	return pvt.eyePos;
}

void Gear::Core::MainCamera::setProj(const float fov, const float aspectRatio, const float zNear, const float zFar)
{
	setProj(DirectX::XMMatrixPerspectiveFovLH(fov, aspectRatio, zNear, zFar));
}

void Gear::Core::MainCamera::setView(const DirectX::XMVECTOR& eyePos, const DirectX::XMVECTOR& focus, const DirectX::XMVECTOR& up)
{
	pvt.eyePos = eyePos;

	setView(DirectX::XMMatrixLookAtLH(eyePos, focus, up));
}

void Gear::Core::MainCamera::setProj(const DirectX::XMMATRIX& proj)
{
	pvt.projMatrix = proj;
}

void Gear::Core::MainCamera::setView(const DirectX::XMMATRIX& view)
{
	pvt.viewMatrix = view;
}