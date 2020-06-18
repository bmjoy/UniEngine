#pragma once
#include "RenderManager.h"
#include "ModelManager.h"
#include "WindowManager.h"
#include "InputManager.h"
#include "LightingManager.h"
namespace UniEngine {
	class UNIENGINE_API EngineDriver {
		World* _World;
		bool _Loopable;
		void DrawInfoWindow();
	public:
		EngineDriver();
		void GLInit();
		void Start();
		bool LoopStart();
		bool Loop();
		bool LoopEnd();
		void End();
		World* GetWorld();
		
	};
}