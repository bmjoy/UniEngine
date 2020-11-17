#pragma once
#include "Core.h"
#include "UniEngineAPI.h"
#include "CameraComponent.h"
#include "MeshRenderer.h"
#include "Particles.h"
#include "RenderTarget.h"

#include "DirectionalLight.h"
#include "PointLight.h"
#include "SpotLight.h"
#include "DirectionalLightShadowMap.h"
#include "PointLightShadowMap.h"

#include "Cubemap.h"
#include "Default.h"
#include "Texture2D.h"


namespace UniEngine {
	struct UNIENGINE_API LightSettings {
		float SplitDistance[4];
		int PCSSPCFSampleAmount = 16;
		float PCSSScaleFactor = 1.0f;
		int PCSSBSAmount = 2;
		float SeamFixRatio = 0.05f;
		float VSMMaxVariance = 0.001f;
		float LightBleedFactor = 0.5f;
		float EVSMExponent = 40.0f;
		float AmbientLight = 0.1f;
	};

	struct MaterialTextures
	{
		GLuint64 diffuse = 0;
		GLuint64 specular = 0;
		GLuint64 ambient = 0;
		GLuint64 emissive = 0;
		GLuint64 height = 0;
		GLuint64 normal = 0;
		GLuint64 directionalShadowMap = 0;
		GLuint64 pointShadowMap = 0;
	};
	
	class UNIENGINE_API RenderManager : public ManagerBase
	{
		static CameraComponent* _MainCameraComponent;
#pragma region Global Var
#pragma region GUI
		static bool _EnableLightMenu;
		static bool _EnableRenderMenu;
		static bool _EnableInfoWindow;
#pragma endregion
#pragma region SSAO
		static bool _EnableSSAO;
		static std::unique_ptr<GLProgram> _SSAOGeometryPass;
		static std::unique_ptr<GLProgram> _SSAOBlurPass;

		static float _SSAOKernelRadius;
		static float _SSAOKernelBias;
		static float _SSAOScale;
		static float _SSAOFactor;
		static int _SSAOSampleSize;
#pragma endregion
#pragma region Render
		static std::unique_ptr<GLUBO> _KernelBlock;
		static std::unique_ptr<GLProgram> _GBufferLightingPass;

		static int _MainCameraResolutionX;
		static int _MainCameraResolutionY;
		static EntityQuery _DirectionalLightQuery;
		static EntityQuery _PointLightQuery;
		static EntityQuery _SpotLightQuery;
		friend class RenderTarget;
		static size_t _Triangles;
		static size_t _DrawCall;
		static MaterialTextures _MaterialTextures;
		static std::unique_ptr<GLUBO> _MaterialTextureBindings;
#pragma endregion
#pragma region Shadow
		static GLUBO* _DirectionalLightBlock;
		static GLUBO* _PointLightBlock;
		static GLUBO* _SpotLightBlock;

		static float _ShadowCascadeSplit[Default::ShaderIncludes::ShadowCascadeAmount];
		static size_t _DirectionalShadowMapResolution;
		static GLUBO* _ShadowCascadeInfoBlock;
		static LightSettings _LightSettings;

		static DirectionalLightInfo _DirectionalLights[Default::ShaderIncludes::MaxDirectionalLightAmount];
		static PointLightInfo _PointLights[Default::ShaderIncludes::MaxPointLightAmount];
		static SpotLightInfo _SpotLights[Default::ShaderIncludes::MaxSpotLightAmount];

		static bool _EnableShadow;

		static GLProgram* _DirectionalLightProgram;
		static GLProgram* _PointLightProgram;
		static GLProgram* _DirectionalLightInstancedProgram;
		static GLProgram* _PointLightInstancedProgram;

		friend class RenderManager;
		static DirectionalLightShadowMap* _DirectionalLightShadowMap;
		static RenderTarget* _DirectionalLightShadowMapFilter;
		static PointLightShadowMap* _PointLightShadowMap;
		static GLProgram* _DirectionalLightVFilterProgram;
		static GLProgram* _DirectionalLightHFilterProgram;


		static GLTexture* _DLVSMVFilter;
		static bool _StableFit;
		static float _MaxShadowDistance;
#pragma endregion
#pragma endregion
		static void MaterialTextureBindHelper(Material* material, std::shared_ptr<GLProgram> program);
		static void DeferredPrepass(Mesh* mesh, Material* material, glm::mat4 model);
		static void DeferredPrepassInstanced(Mesh* mesh, Material* material, glm::mat4 model, glm::mat4* matrices, size_t count);

		static void DrawMeshInstanced(Mesh* mesh, Material* material, glm::mat4 model, glm::mat4* matrices, size_t count, bool receiveShadow);
		static void DrawMesh(Mesh* mesh, Material* material, glm::mat4 model, bool receiveShadow);

		static void DrawGizmoInstanced(Mesh* mesh, glm::vec4 color, glm::mat4 model, glm::mat4* matrices, size_t count, glm::mat4 scaleMatrix);
		static void DrawGizmo(Mesh* mesh, glm::vec4 color, glm::mat4 model, glm::mat4 scaleMatrix);

		
		static void DrawTexture2D(GLTexture2D* texture, float depth, glm::vec2 center, glm::vec2 size);
		static float Lerp(float a, float b, float f);

		static void DrawMesh(Mesh* mesh, Material* material, glm::mat4 model, RenderTarget* target, bool receiveShadow = true);
		static void DrawMeshInstanced(Mesh* mesh, Material* material, glm::mat4 model, glm::mat4* matrices, size_t count, RenderTarget* target, bool receiveShadow = true);

		
	public:
#pragma region Settings
		static void SetSSAOKernelRadius(float value);
		static void SetSSAOKernelBias(float value);
		static void SetSSAOScale(float value);
		static void SetSSAOFactor(float value);
		static void SetEnableSSAO(bool value);
		static void SetSSAOSampleSize(int value);
#pragma endregion
		static void RenderToCameraDeferred(std::unique_ptr<CameraComponent>& cameraComponent, LocalToWorld& cameraTransform, glm::vec3& minBound, glm::vec3& maxBound, bool calculateBounds = false);
		static void RenderBackGround(std::unique_ptr<CameraComponent>& cameraComponent);
		static void RenderToCameraForward(std::unique_ptr<CameraComponent>& cameraComponent, LocalToWorld& cameraTransform, glm::vec3& minBound, glm::vec3& maxBound, bool calculateBounds = false);
		static void Init();
		//Main rendering happens here.
		static void PreUpdate();
#pragma region Shadow
		static void SetSplitRatio(float r1, float r2, float r3, float r4);
		static void SetDirectionalLightResolution(size_t value);
		static void SetPCSSPCFSampleAmount(int value);
		static void SetPCSSBSAmount(int value);
		static void SetStableFit(bool value);
		static void SetSeamFixRatio(float value);
		static void SetMaxShadowDistance(float value);
		static void SetPCSSScaleFactor(float value);
		static void SetVSMMaxVariance(float value);
		static void SetLightBleedControlFactor(float value);
		static void SetEVSMExponent(float value);
		static void SetAmbientLight(float value);
		static void SetEnableShadow(bool value);
		static glm::vec3 ClosestPointOnLine(glm::vec3 point, glm::vec3 a, glm::vec3 b);
#pragma endregion
#pragma region RenderAPI
		static void LateUpdate();
		static size_t Triangles();
		static size_t DrawCall();

		static void DrawTexture2D(GLTexture2D* texture, float depth, glm::vec2 center, glm::vec2 size, RenderTarget* target);
		static void DrawTexture2D(Texture2D* texture, float depth, glm::vec2 center, glm::vec2 size, RenderTarget* target);
		static void DrawTexture2D(Texture2D* texture, float depth, float centerX, float centerY, float sizeX, float sizeY, RenderTarget* target);

		static void SetMainCamera(CameraComponent* value);
		static CameraComponent* GetMainCamera();
		static void DrawGizmoPoint(glm::vec4 color, glm::mat4 model = glm::mat4(1.0f), float size = 1.0f);
		static void DrawGizmoPointInstanced(glm::vec4 color, glm::mat4* matrices, size_t count, glm::mat4 model = glm::mat4(1.0f), float size = 1.0f);
		static void DrawGizmoCube(glm::vec4 color, glm::mat4 model = glm::mat4(1.0f), float size = 1.0f);
		static void DrawGizmoCubeInstanced(glm::vec4 color, glm::mat4* matrices, size_t count, glm::mat4 model = glm::mat4(1.0f), float size = 1.0f);
		static void DrawGizmoQuad(glm::vec4 color, glm::mat4 model = glm::mat4(1.0f), float size = 1.0f);
		static void DrawGizmoQuadInstanced(glm::vec4 color, glm::mat4* matrices, size_t count, glm::mat4 model = glm::mat4(1.0f), float size = 1.0f);
		static void DrawGizmoMesh(Mesh* mesh, glm::vec4 color, glm::mat4 model = glm::mat4(1.0f), float size = 1.0f);
		static void DrawGizmoMeshInstanced(Mesh* mesh, glm::vec4 color, glm::mat4* matrices, size_t count, glm::mat4 model = glm::mat4(1.0f), float size = 1.0f);

		static void DrawGizmoRay(glm::vec4 color, glm::vec3 start, glm::vec3 end, float width = 0.01f);
		static void DrawGizmoRay(glm::vec4 color, Ray& ray, float width = 0.01f);

		static void DrawMesh(Mesh* mesh, Material* material, glm::mat4 model, CameraComponent* cameraComponent, bool receiveShadow = true);
		static void DrawMeshInstanced(Mesh* mesh, Material* material, glm::mat4 model, glm::mat4* matrices, size_t count, CameraComponent* cameraComponent, bool receiveShadow = true);

#pragma endregion
	};

	
}
