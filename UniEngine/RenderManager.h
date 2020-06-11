#pragma once
#include "Core.h"
#include "ManagerBase.h"
#include "CameraComponent.h"
#include "MeshMaterialComponent.h"
#include "InstancedMeshMaterialComponent.h"
#include "RenderTarget.h"



#include "Cubemap.h"
#include "Texture2D.h"


namespace UniEngine {
	class UECORE_API RenderManager : public ManagerBase
	{
		static GLenum _TextureStartIndex;



		friend class RenderTarget;
		static RenderTarget* _CurrentRenderTarget;
		static unsigned _Triangles;
		static unsigned _DrawCall;

		static void DrawMeshInstanced(Mesh* mesh, Material* material, glm::mat4 matrix, glm::mat4* matrices, size_t count);
		static void DrawMesh(Mesh* mesh, Material* material, glm::mat4 matrix);
	public:
		static void Start();
		static unsigned Triangles();
		static unsigned DrawCall();

		static void DrawMeshInstanced(InstancedMeshMaterialComponent* immc, glm::mat4 matrix, glm::mat4* matrices, size_t count, Camera* camera);
		static void DrawMesh(MeshMaterialComponent* mmc, glm::mat4 matrix, Camera* camera);

		static void DrawMeshInstanced(Mesh* mesh, Material* material, glm::mat4 matrix, glm::mat4* matrices, size_t count, Camera* camera);
		static void DrawMesh(Mesh* mesh, Material* material, glm::mat4 matrix, Camera* camera);

		static void DrawMeshInstanced(Mesh* mesh, Material* material, glm::mat4 matrix, glm::mat4* matrices, size_t count, RenderTarget* target);
		static void DrawMesh(Mesh* mesh, Material* material, glm::mat4 matrix, RenderTarget* target);

	};
}
