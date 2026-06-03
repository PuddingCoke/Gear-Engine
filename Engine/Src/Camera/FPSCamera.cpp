#include<Gear/Camera/FPSCamera.h>

#include<Gear/Core/MainCamera.h>

#include<Gear/Input/Mouse.h>

#include<Gear/Input/Keyboard.h>

namespace Gear
{
	FPSCamera::FPSCamera(const DirectX::XMVECTOR& eye, const DirectX::XMVECTOR& lookDir, const DirectX::XMVECTOR& up, const float moveSpeed) :
		eye(eye), lookDir(lookDir), up(up), moveSpeed(moveSpeed)
	{
		this->lookDir = DirectX::XMVector3Normalize(this->lookDir);

		moveEventID = Input::Mouse::addMoveEvent([this]()
			{
				if (Input::Mouse::getLeftDown())
				{
					const DirectX::XMMATRIX rotMat = DirectX::XMMatrixRotationAxis(this->up, Input::Mouse::getDX() / 120.f);

					this->lookDir = DirectX::XMVector3Transform(this->lookDir, rotMat);

					const DirectX::XMVECTOR upCrossLookDir = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(this->up, this->lookDir));

					float lookUpAngle;

					DirectX::XMStoreFloat(&lookUpAngle, DirectX::XMVector3AngleBetweenNormals(this->lookDir, this->up));

					const float destAngle = lookUpAngle - Input::Mouse::getDY() / 120.f;

					float rotAngle = -Input::Mouse::getDY() / 120.f;

					if (destAngle > Utils::Math::pi - Core::MainCamera::epsilon)
					{
						rotAngle = Utils::Math::pi - Core::MainCamera::epsilon - lookUpAngle;
					}
					else if (destAngle < Core::MainCamera::epsilon)
					{
						rotAngle = Core::MainCamera::epsilon - lookUpAngle;
					}

					const DirectX::XMMATRIX lookDirRotMat = DirectX::XMMatrixRotationAxis(upCrossLookDir, rotAngle);

					this->lookDir = DirectX::XMVector3Transform(this->lookDir, lookDirRotMat);
				}
			});
	}

	FPSCamera::~FPSCamera()
	{
		Input::Mouse::removeMoveEvent(moveEventID);
	}

	void FPSCamera::applyInput(const float dt)
	{
		if (Input::Keyboard::getKeyDown(Input::Keyboard::W))
		{
			eye = DirectX::XMVectorAdd(eye, DirectX::XMVectorScale(lookDir, dt * moveSpeed));
		}

		if (Input::Keyboard::getKeyDown(Input::Keyboard::S))
		{
			eye = DirectX::XMVectorAdd(eye, DirectX::XMVectorScale(lookDir, -dt * moveSpeed));
		}

		if (Input::Keyboard::getKeyDown(Input::Keyboard::A))
		{
			const DirectX::XMVECTOR upCrossLookDir = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(up, lookDir));
			eye = DirectX::XMVectorAdd(eye, DirectX::XMVectorScale(upCrossLookDir, -dt * moveSpeed));
		}

		if (Input::Keyboard::getKeyDown(Input::Keyboard::D))
		{
			const DirectX::XMVECTOR upCrossLookDir = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(up, lookDir));
			eye = DirectX::XMVectorAdd(eye, DirectX::XMVectorScale(upCrossLookDir, dt * moveSpeed));
		}

		Core::MainCamera::setView(eye, DirectX::XMVectorAdd(eye, lookDir), up);
	}

	DirectX::XMVECTOR FPSCamera::getEyePos() const
	{
		return eye;
	}

	DirectX::XMVECTOR FPSCamera::getLookDir() const
	{
		return lookDir;
	}

	DirectX::XMVECTOR FPSCamera::getUpVector() const
	{
		return up;
	}
}
