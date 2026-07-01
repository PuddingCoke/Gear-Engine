#include<Gear/Camera/OrbitCamera.h>

#include<Gear/Core/MainCamera.h>

#include<Gear/Input/Mouse.h>

#include<Gear/Input/Keyboard.h>

namespace Gear
{
	namespace Camera
	{
		OrbitCamera::OrbitCamera(const DirectX::XMVECTOR& eye, const DirectX::XMVECTOR& up, const float zoomSpeed) :
			eye(DirectX::XMVector3Normalize(eye)), up(DirectX::XMVector3Normalize(up)), zoomSpeed(zoomSpeed)
		{
			DirectX::XMStoreFloat(&targetRadius, DirectX::XMVector3Length(eye));

			currentRadius = targetRadius;

			moveEventID = Input::Mouse::addMoveEvent([this]()
				{
					if (Input::Mouse::getLeftDown())
					{
						const DirectX::XMMATRIX rotMat = DirectX::XMMatrixRotationAxis(this->up, Input::Mouse::getDX() / 120.f);

						this->eye = DirectX::XMVector3Transform(this->eye, rotMat);

						float eyeUpAngle;

						DirectX::XMStoreFloat(&eyeUpAngle, DirectX::XMVector3AngleBetweenNormals(this->eye, this->up));

						const float destAngle = eyeUpAngle + Input::Mouse::getDY() / 120.f;

						float rotAngle = Input::Mouse::getDY() / 120.f;

						if (destAngle > Utils::Math::pi - Core::MainCamera::epsilon)
						{
							rotAngle = Utils::Math::pi - Core::MainCamera::epsilon - eyeUpAngle;
						}
						else if (destAngle < Core::MainCamera::epsilon)
						{
							rotAngle = Core::MainCamera::epsilon - eyeUpAngle;
						}

						const DirectX::XMVECTOR upCrossLookDir = DirectX::XMVector3Cross(this->up, this->eye);

						const DirectX::XMMATRIX upRotMat = DirectX::XMMatrixRotationAxis(upCrossLookDir, rotAngle);

						this->eye = DirectX::XMVector3Transform(this->eye, upRotMat);
					}
				});

			scrollEventID = Input::Mouse::addScrollEvent([this]()
				{
					targetRadius -= 0.5f * this->zoomSpeed * Input::Mouse::getWheelDelta();

					if (targetRadius < 0.1f)
					{
						targetRadius = 0.1f;
					}
				});
		}

		OrbitCamera::~OrbitCamera()
		{
			Input::Mouse::removeMoveEvent(moveEventID);

			Input::Mouse::removeScrollEvent(scrollEventID);
		}

		void OrbitCamera::applyInput(const float dt)
		{
			currentRadius = Utils::Math::lerp(currentRadius, targetRadius, Utils::Math::clamp(dt * 20.f, 0.f, 1.f));

			Core::MainCamera::setView(DirectX::XMVectorScale(eye, currentRadius), { 0,0,0 }, up);
		}

		void OrbitCamera::rotateX(const float dTheta)
		{
			const DirectX::XMMATRIX rotMat = DirectX::XMMatrixRotationAxis(up, dTheta);

			eye = DirectX::XMVector3Transform(eye, rotMat);
		}

		void OrbitCamera::rotateY(const float dTheta)
		{
			float eyeUpAngle;

			DirectX::XMStoreFloat(&eyeUpAngle, DirectX::XMVector3AngleBetweenNormals(eye, up));

			const float destAngle = eyeUpAngle + dTheta;

			float rotAngle = dTheta;

			if (destAngle > Utils::Math::pi - Core::MainCamera::epsilon)
			{
				rotAngle = Utils::Math::pi - Core::MainCamera::epsilon - eyeUpAngle;
			}
			else if (destAngle < Core::MainCamera::epsilon)
			{
				rotAngle = Core::MainCamera::epsilon - eyeUpAngle;
			}

			const DirectX::XMVECTOR upCrossLookDir = DirectX::XMVector3Cross(up, eye);

			const DirectX::XMMATRIX upRotMat = DirectX::XMMatrixRotationAxis(upCrossLookDir, rotAngle);

			eye = DirectX::XMVector3Transform(eye, upRotMat);
		}
	}
}
