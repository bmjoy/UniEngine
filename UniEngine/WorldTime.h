#pragma once
#include "UniEngineAPI.h"
namespace UniEngine {
	class UNIENGINE_API WorldTime {
	private:
		friend class World;
		double _WorldTime;
		double _FixedDeltaTime;
		double _DeltaTime;
		double _LastFrameTime;
		float _TimeStep;
		void AddFixedDeltaTime(double value);
	public:
		float TimeStep();
		float FixedDeltaTime();
		float DeltaTime();
		float LastFrameTime();
	};
}