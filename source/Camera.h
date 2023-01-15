#pragma once
#include <cassert>
#include <SDL_keyboard.h>
#include <SDL_mouse.h>

#include "Math.h"
#include "Timer.h"

namespace dae
{
	struct Camera
	{
		Camera() = default;

		Camera(const Vector3& _origin, float _fovAngle, float aspectRatio):
			origin{_origin},
			fovAngle{_fovAngle},
			aspectRatio{aspectRatio}
		{
		}


		Vector3 origin{};
		float fovAngle{90.f};
		float fov{ tanf((fovAngle * TO_RADIANS) / 2.f) };
		const float maxFovAngle{ 179.f };
		const float minFovAngle{ 1.f };

		float aspectRatio{};

		float nearPlane{ 0.1f };
		float farPlane{ 100.f };

		Vector3 forward{Vector3::UnitZ};
		Vector3 up{Vector3::UnitY};
		Vector3 right{Vector3::UnitX};

		float totalPitch{};
		float totalYaw{};

		Matrix invViewMatrix{};
		Matrix viewMatrix{};
		Matrix projectionMatrix{};

		void Initialize(float _fovAngle = 90.f, Vector3 _origin = {0.f,0.f,0.f}, float _aspectRatio = 1.f)
		{
			fovAngle = _fovAngle;
			fov = tanf((fovAngle * TO_RADIANS) / 2.f);

			origin = _origin;

			aspectRatio = _aspectRatio;
		}

		void CalculateViewMatrix()
		{
			//DirectX Implementation => https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3dxmatrixlookatlh

			Matrix rotationMatrix{ Matrix::CreateRotation(totalPitch, totalYaw, 0.f) };
			forward = rotationMatrix.TransformVector(Vector3::UnitZ).Normalized();
			right = Vector3::Cross(Vector3::UnitY, forward).Normalized();
			up = Vector3::Cross(forward, right).Normalized();

			viewMatrix = Matrix::CreateLookAtLH(origin, forward, Vector3::UnitY);
			invViewMatrix = Matrix::Inverse(viewMatrix);
		}

		void CalculateProjectionMatrix()
		{
			//ProjectionMatrix => Matrix::CreatePerspectiveFovLH(...) [not implemented yet]
			projectionMatrix = Matrix::CreatePerspectiveFovLH(fov, aspectRatio, nearPlane, farPlane);
			//DirectX Implementation => https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3dxmatrixperspectivefovlh
		}

		void Update(const Timer* pTimer)
		{
			const float deltaTime = pTimer->GetElapsed();

			//Camera Update Logic
			//...
			float cameraMovementSpeed{ 20.f };
			float cameraRotationSpeed{ 180.f * TO_RADIANS };
			const float elapsedSec{ pTimer->GetElapsed() };

			//Keyboard Input
			const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);

			if (pKeyboardState[SDL_SCANCODE_LSHIFT])
			{
				cameraMovementSpeed *= 4;
				cameraRotationSpeed *= 4;
			}

			if (pKeyboardState[SDL_SCANCODE_W] || pKeyboardState[SDL_SCANCODE_UP])
			{
				origin += forward * cameraMovementSpeed * elapsedSec;
			}

			if (pKeyboardState[SDL_SCANCODE_S] || pKeyboardState[SDL_SCANCODE_DOWN])
			{
				origin += -forward * cameraMovementSpeed * elapsedSec;
			}

			if (pKeyboardState[SDL_SCANCODE_D] || pKeyboardState[SDL_SCANCODE_RIGHT])
			{
				origin += right * cameraMovementSpeed * elapsedSec;
			}

			if (pKeyboardState[SDL_SCANCODE_A] || pKeyboardState[SDL_SCANCODE_LEFT])
			{
				origin += -right * cameraMovementSpeed * elapsedSec;
			}

			//change fov
			if (pKeyboardState[SDL_SCANCODE_Q] && fovAngle > minFovAngle)
			{
				fovAngle -= 1.f;
			}

			if (pKeyboardState[SDL_SCANCODE_E] && fovAngle < maxFovAngle)
			{
				fovAngle += 1.f;
			}

			//Mouse Input
			int mouseX{}, mouseY{};
			const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);

			if ((mouseState & SDL_BUTTON_RMASK) && (mouseState & SDL_BUTTON_LMASK))
			{
				if (mouseY > 0)
				{
					origin += -up * cameraMovementSpeed * elapsedSec;
				}

				if (mouseY < 0)
				{
					origin += up * cameraMovementSpeed * elapsedSec;
				}

				if (mouseX > 0)
				{
					origin += right * cameraMovementSpeed * elapsedSec;
				}

				if (mouseX < 0)
				{
					origin += -right * cameraMovementSpeed * elapsedSec;
				}
			}
			else if (mouseState & SDL_BUTTON_LMASK)
			{
				if (mouseY > 0)
				{
					origin += forward * -cameraMovementSpeed * elapsedSec;
				}

				if (mouseY < 0)
				{
					origin += forward * cameraMovementSpeed * elapsedSec;
				}

				if (mouseX > 0)
				{
					totalYaw += cameraRotationSpeed * elapsedSec;
				}

				if (mouseX < 0)
				{
					totalYaw -= cameraRotationSpeed * elapsedSec;
				}
			}
			else if (mouseState & SDL_BUTTON_RMASK)
			{
				if (mouseX > 0)
				{
					totalYaw += cameraRotationSpeed * elapsedSec;
				}

				if (mouseX < 0)
				{
					totalYaw -= cameraRotationSpeed * elapsedSec;
				}

				if (mouseY > 0)
				{
					totalPitch -= cameraRotationSpeed * elapsedSec;
				}

				if (mouseY < 0)
				{
					totalPitch += cameraRotationSpeed * elapsedSec;
				}
			}

			//Update Matrices
			CalculateViewMatrix();
			CalculateProjectionMatrix(); //Try to optimize this - should only be called once or when fov/aspectRatio changes
		}
	};
}
