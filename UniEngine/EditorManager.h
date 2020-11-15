#pragma once
#include "CameraComponent.h"
#include "Core.h"
#include "UniEngineAPI.h"
namespace UniEngine {
	enum EntityEditorSystemConfigFlags {
		EntityEditorSystem_None = 0,
		EntityEditorSystem_EnableEntityHierarchy = 1 << 0,
		EntityEditorSystem_EnableEntityInspector = 1 << 1
	};
	class UNIENGINE_API EditorManager :
		public ManagerBase
	{
		static EntityArchetype _BasicEntityArchetype;
		static bool _Enabled;
		static std::map<size_t, std::function<void(ComponentBase* data, bool isRoot)>> _ComponentDataInspectorMap;
		static std::vector<std::pair<size_t, std::function<void(Entity owner)>>> _PrivateComponentMenuList;
		static std::vector<std::pair<size_t, std::function<void(Entity owner)>>> _ComponentDataMenuList;
		static unsigned int _ConfigFlags;
		static int _SelectedHierarchyDisplayMode;
		static Entity _SelectedEntity;
		static bool _DisplayLog;
		static bool _DisplayError;

#pragma region Scene Camera
		friend class RenderManager;
		friend class InputManager;
		static glm::quat _SceneCameraRotation;
		static glm::vec3 _SceneCameraPosition;
		static std::unique_ptr<CameraComponent> _SceneCamera;
		static int _SceneCameraResolutionX;
		static int _SceneCameraResolutionY;
		static float _Velocity;
		static float _Sensitivity;
		static float _LastX;
		static float _LastY;
		static float _LastScrollY;
		static bool _StartMouse;
		static bool _StartScroll;
#pragma endregion
		static bool DrawEntityMenu(bool enabled, Entity& entity);
		static void DrawEntityNode(Entity& entity);
		static void InspectComponentData(ComponentBase* data, ComponentType type, bool isRoot);
	public:
		static void LateUpdate();
		template<typename T1 = ComponentBase>
		static void RegisterComponentDataInspector(const std::function<void(ComponentBase* data, bool isRoot)>& func);
		template<typename T1 = PrivateComponentBase>
		static void RegisterPrivateComponentMenu(const std::function<void(Entity owner)>& func);
		template<typename T1 = ComponentBase>
		static void RegisterComponentDataMenu(const std::function<void(Entity owner)>& func);
		static void Init();
		static void Destroy();
		static void PreUpdate();
		static void Update();
		static Entity GetSelectedEntity() { return _SelectedEntity; }
		static void SetSelectedEntity(Entity entity);
	};

	template <typename T1>
	void EditorManager::RegisterComponentDataInspector(const std::function<void(ComponentBase* data, bool isRoot)>& func)
	{
		_ComponentDataInspectorMap.insert_or_assign(typeid(T1).hash_code(), func);
	}

	template <typename T1>
	void EditorManager::RegisterPrivateComponentMenu(const std::function<void(Entity owner)>& func)
	{
		for(int i = 0; i < _PrivateComponentMenuList.size(); i++)
		{
			if(_PrivateComponentMenuList[i].first == typeid(T1).hash_code())
			{
				_PrivateComponentMenuList[i].second = func;
				return;
			}
		}
		_PrivateComponentMenuList.emplace_back(typeid(T1).hash_code(), func);
	}

	template <typename T1>
	void EditorManager::RegisterComponentDataMenu(const std::function<void(Entity owner)>& func)
	{
		for (int i = 0; i < _ComponentDataMenuList.size(); i++)
		{
			if (_ComponentDataMenuList[i].first == typeid(T1).hash_code())
			{
				_ComponentDataMenuList[i].second = func;
				return;
			}
		}
		_ComponentDataMenuList.emplace_back(typeid(T1).hash_code(), func);
	}
}
