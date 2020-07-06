#pragma once
#include "UniEngineAPI.h"
namespace UniEngine {
	class UNIENGINE_API TransformSystem :
		public SystemBase
	{
	public:
		TransformSystem();
		void OnCreate();
		void OnDestroy();
		void Update();
	};

}