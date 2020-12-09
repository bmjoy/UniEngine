﻿#pragma once
#include "Core.h"
#include "UniEngineAPI.h"
namespace UniEngine {
	class UNIENGINE_API RenderTarget
	{
		bool _Bound;
	protected:
		std::unique_ptr<GLFrameBuffer> _FrameBuffer;
		size_t _ResolutionX;
		size_t _ResolutionY;
	public:
		void AttachTextureLayer(GLTexture* texture, GLenum attachPoint, GLint layer);
		void AttachTexture(GLTexture* texture, GLenum attachPoint);
		void AttachRenderBuffer(GLRenderBuffer* renderBuffer, GLenum attachPoint);
		RenderTarget();
		RenderTarget(size_t width, size_t height);
		glm::vec2 GetResolution() const;
		void SetResolution(size_t width, size_t height);
		float GetResolutionRatio() const;
		void Bind();
		void Clear();

		std::unique_ptr<GLFrameBuffer>& GetFrameBuffer()
		{
			return _FrameBuffer;
		}

		static void BindDefault();
	};

}