#pragma once
#include "UniEngineAPI.h"
#include "CameraComponent.h"
#include "MeshMaterialComponent.h"
#include "InstancedMeshMaterialComponent.h"
#include "RenderTarget.h"



#include "Cubemap.h"
#include "Texture2D.h"


namespace UniEngine {
	class UNIENGINE_API RenderManager : public ManagerBase
	{
		friend class RenderTarget;
		static unsigned _Triangles;
		static unsigned _DrawCall;
		static bool _EnableNormalMapping;
		static void DrawMeshInstanced(Mesh* mesh, Material* material, glm::mat4 matrix, glm::mat4* matrices, size_t count, bool receiveShadow);
		static void DrawMesh(Mesh* mesh, Material* material, glm::mat4 matrix, bool receiveShadow);
		static void DrawTexture2D(GLTexture2D* texture, float depth, glm::vec2 center, glm::vec2 size);
	public:
		static void SetEnableNormalMapping(bool value);
		static void Start();
		static unsigned Triangles();
		static unsigned DrawCall();

		static void DrawTexture2D(GLTexture2D* texture, float depth, glm::vec2 center, glm::vec2 size, RenderTarget* target);
		static void DrawTexture2D(Texture2D* texture, float depth, glm::vec2 center, glm::vec2 size, RenderTarget* target);
		static void DrawTexture2D(Texture2D* texture, float depth, glm::vec2 center, glm::vec2 size, Camera* camera);
		static void DrawTexture2D(Texture2D* texture, float depth, float centerX, float centerY, float sizeX, float sizeY, Camera* camera);

		static void DrawMesh(MeshMaterialComponent* mmc, glm::mat4 matrix, Camera* camera);
		static void DrawMesh(Mesh* mesh, Material* material, glm::mat4 matrix, Camera* camera, bool receiveShadow = true);
		static void DrawMesh(Mesh* mesh, Material* material, glm::mat4 matrix, RenderTarget* target, bool receiveShadow = true);

		static void DrawMeshInstanced(InstancedMeshMaterialComponent* immc, glm::mat4 matrix, Camera* camera);
		static void DrawMeshInstanced(Mesh* mesh, Material* material, glm::mat4 matrix, glm::mat4* matrices, size_t count, Camera* camera, bool receiveShadow = true);
		static void DrawMeshInstanced(Mesh* mesh, Material* material, glm::mat4 matrix, glm::mat4* matrices, size_t count, RenderTarget* target, bool receiveShadow = true);
	};
}
