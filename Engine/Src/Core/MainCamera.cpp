#include<Gear/Core/MainCamera.h>

namespace
{
	struct MainCameraImpl
	{

		DirectX::XMMATRIX projMatrix = {};

		DirectX::XMMATRIX viewMatrix = {};

		DirectX::XMVECTOR eyePos = {};

	}impl;
}

DirectX::XMMATRIX Gear::Core::MainCamera::getProj()
{
	return impl.projMatrix;
}

DirectX::XMMATRIX Gear::Core::MainCamera::getView()
{
	return impl.viewMatrix;
}

DirectX::XMVECTOR Gear::Core::MainCamera::getEyePos()
{
	return impl.eyePos;
}

void Gear::Core::MainCamera::setProj(const float fov, const float aspectRatio, const float zNear, const float zFar)
{
	setProj(DirectX::XMMatrixPerspectiveFovLH(fov, aspectRatio, zNear, zFar));
}

void Gear::Core::MainCamera::setView(const DirectX::XMVECTOR& eyePos, const DirectX::XMVECTOR& focus, const DirectX::XMVECTOR& up)
{
	impl.eyePos = eyePos;

	setView(DirectX::XMMatrixLookAtLH(eyePos, focus, up));
}

void Gear::Core::MainCamera::setProj(const DirectX::XMMATRIX& proj)
{
	impl.projMatrix = proj;
}

void Gear::Core::MainCamera::setView(const DirectX::XMMATRIX& view)
{
	impl.viewMatrix = view;
}