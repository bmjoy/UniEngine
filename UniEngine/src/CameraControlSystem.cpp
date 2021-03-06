#include "pch.h"
#include "CameraControlSystem.h"
#include "InputManager.h"
#include "UniEngine.h"
using namespace UniEngine;
void CameraControlSystem::LateUpdate()
{
	auto* mainCamera = RenderManager::GetMainCamera();
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
	ImGui::Begin("Camera");
	{
		// Using a Child allow to fill all the space of the window.
		// It also allows customization
		if (ImGui::BeginChild("CameraRenderer")) {
			if (ImGui::IsWindowFocused())
			{
#pragma region Scene Camera Controller
				auto transform = mainCamera->GetOwner().GetComponentData<Transform>();
				auto rotation = transform.GetRotation();
				auto position = transform.GetPosition();
				glm::vec3 front = rotation * glm::vec3(0, 0, -1);
				glm::vec3 right = rotation * glm::vec3(1, 0, 0);
				bool moved = false;
				if (InputManager::GetKey(GLFW_KEY_W)) {
					position += glm::vec3(front.x, 0.0f, front.z) * (float)Application::GetCurrentWorld()->Time()->DeltaTime() * _Velocity;
					moved = true;
				}
				if (InputManager::GetKey(GLFW_KEY_S)) {
					position -= glm::vec3(front.x, 0.0f, front.z) * (float)Application::GetCurrentWorld()->Time()->DeltaTime() * _Velocity;
					moved = true;
				}
				if (InputManager::GetKey(GLFW_KEY_A)) {
					position -= glm::vec3(right.x, 0.0f, right.z) * (float)Application::GetCurrentWorld()->Time()->DeltaTime() * _Velocity;
					moved = true;
				}
				if (InputManager::GetKey(GLFW_KEY_D)) {
					position += glm::vec3(right.x, 0.0f, right.z) * (float)Application::GetCurrentWorld()->Time()->DeltaTime() * _Velocity;
					moved = true;
				}
				if (InputManager::GetKey(GLFW_KEY_LEFT_SHIFT)) {
					position.y += _Velocity * (float)Application::GetCurrentWorld()->Time()->DeltaTime();
					moved = true;
				}
				if (InputManager::GetKey(GLFW_KEY_LEFT_CONTROL)) {
					position.y -= _Velocity * (float)Application::GetCurrentWorld()->Time()->DeltaTime();
					moved = true;
				}
				if (moved)
				{
					transform.SetPosition(position);
				}
				glm::vec2 mousePosition;
				bool valid = InputManager::GetMousePosition(mousePosition);
				float xOffset = 0;
				float yOffset = 0;
				if (valid) {
					if (!_StartMouse) {
						_LastX = mousePosition.x;
						_LastY = mousePosition.y;
						_StartMouse = true;
					}
					xOffset = mousePosition.x - _LastX;
					yOffset = -mousePosition.y + _LastY;
					_LastX = mousePosition.x;
					_LastY = mousePosition.y;
				}
				if (InputManager::GetMouse(GLFW_MOUSE_BUTTON_RIGHT)) {
					if (xOffset != 0 || yOffset != 0) {
						moved = true;
						_SceneCameraYawAngle += xOffset * _Sensitivity;
						_SceneCameraPitchAngle += yOffset * _Sensitivity;

						if (_SceneCameraPitchAngle > 89.0f)
							_SceneCameraPitchAngle = 89.0f;
						if (_SceneCameraPitchAngle < -89.0f)
							_SceneCameraPitchAngle = -89.0f;

						transform.SetRotation(CameraComponent::ProcessMouseMovement(_SceneCameraYawAngle, _SceneCameraPitchAngle, false));
					}
				}
				if (moved)
				{
					EntityManager::SetComponentData(mainCamera->GetOwner(), transform);
				}
#pragma endregion
			}
		}
		ImGui::EndChild();
	}
	ImGui::End();
	ImGui::PopStyleVar();
}



void CameraControlSystem::SetVelocity(float velocity)
{
	_Velocity = velocity;
}

void CameraControlSystem::SetSensitivity(float sensitivity)
{
	_Sensitivity = sensitivity;
}
