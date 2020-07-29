#pragma once
#include "UniEngineAPI.h"
#include "CameraComponent.h"

namespace UniEngine {
    class UNIENGINE_API CameraControlSystem :
        public SystemBase
    {
		Entity _TargetCameraEntity;
		CameraComponent* _TargetCameraComponent;
		Camera* _TargetCamera;
		float _Velocity;
		float _Sensitivity;
		bool _EnableWindowControl;
		Translation _CameraPosition;
		Rotation _CameraRotation;
		float _LastX = 0, _LastY = 0, _LastScrollY = 0;
		bool startMouse = false;
		bool startScroll = false;
		float _P[3];
		float _R[4];
	public:
		void Update();
		void EnableWindowControl(bool value);
		void SetTargetCamera(Entity targetCameraEntity);
		void SetPosition(glm::vec3 position);
		void SetVelocity(float velocity);
		void SetSensitivity(float sensitivity);
    };
}
