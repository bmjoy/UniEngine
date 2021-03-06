#pragma once
#include "UniEngineAPI.h"
namespace UniEngine {
	class ThreadPool;
	class World;
	class WorldTime;
	class UNIENGINE_API SystemBase
	{
	protected:
		friend class World;
		bool _Enabled;
		WorldTime* _Time;
		World* _World;
		virtual void OnStartRunning() {}
		virtual void OnStopRunning() {}
		SystemBase();
	public:
		void Enable();
		void Disable();
		bool Enabled() const;

		virtual void OnCreate() {}
		virtual void OnDestroy() {}
		virtual void PreUpdate() {}
		virtual void Update() {}
		virtual void FixedUpdate() {}
		virtual void LateUpdate() {}
	};
}