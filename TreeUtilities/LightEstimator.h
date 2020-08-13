#pragma once
#include "TreeUtilitiesAPI.h"
using namespace UniEngine;
namespace TreeUtilities {
	class LightSnapShot {
		GLTexture2D* _SnapShotTexture;
		GLPPBO* _PPBO;
		float* _SRC;
		float _Angle;
		float _Weight;
		float _Score;
		size_t _Resolution;
	public:
		GLTexture2D* SnapShotTexture() { return _SnapShotTexture; }
		GLPPBO* GetPixelBuffer() { return _PPBO; }
		float GetAngle() { return _Angle; };
		float* GetSRC() { return _SRC; }
		LightSnapShot(size_t resolution, float angle, float weight);
		float CalculateScore();
		~LightSnapShot();
	};

	class LightEstimator
	{
		RenderTarget* _RenderTarget;
		size_t _Resolution;
		std::vector<LightSnapShot*> _SnapShots;
		GLProgram* _SnapShotProgram;
		GLRenderBuffer* _DepthBuffer;
		float _LightEstimationScore;
		friend class TreeSystem;
		float _SnapShotWidth = 100.0f;
	public:
		LightEstimator(size_t resolution = 2048);
		void TakeSnapShot(Entity treeEntity, bool calculateScore = false);
		void DrawSnapShots(Camera* camera);
		float CalculateScore();
		float GetScore();
		std::vector<LightSnapShot*>* GetSnapShots();
	};

}