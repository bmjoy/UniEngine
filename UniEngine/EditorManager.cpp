#include "pch.h"
#include "EditorManager.h"

#include "AssetManager.h"
#include "UniEngine.h"
#include "Default.h"
#include "DirectionalLight.h"
#include "imgui_internal.h"
#include "InputManager.h"
#include "Model.h"
#include "PointLight.h"
#include "RenderManager.h"
#include "TransformManager.h"
#include "ImGuizmo/ImGuizmo.h"
using namespace UniEngine;
bool EditorManager::_Enabled = false;
EntityArchetype EditorManager::_BasicEntityArchetype;
std::map<size_t, std::function<void(ComponentBase* data, bool isRoot)>> EditorManager::_ComponentDataInspectorMap;
std::vector<std::pair<size_t, std::function<void(Entity owner)>>> EditorManager::_PrivateComponentMenuList;
std::vector<std::pair<size_t, std::function<void(Entity owner)>>> EditorManager::_ComponentDataMenuList;
unsigned int EditorManager::_ConfigFlags = 0;
int EditorManager::_SelectedHierarchyDisplayMode = 1;
Entity EditorManager::_SelectedEntity;
bool EditorManager::_DisplayLog = true;
bool EditorManager::_DisplayError = true;
Transform* EditorManager::_PreviouslyStoredTransform;
glm::vec3 EditorManager::_PreviouslyStoredPosition;
glm::vec3 EditorManager::_PreviouslyStoredRotation;
glm::vec3 EditorManager::_PreviouslyStoredScale;
glm::quat EditorManager::_SceneCameraRotation;
glm::vec3 EditorManager::_SceneCameraPosition;
std::unique_ptr<CameraComponent> EditorManager::_SceneCamera;
int EditorManager::_SceneCameraResolutionX;
int EditorManager::_SceneCameraResolutionY;
float EditorManager::_Velocity = 20.0f;
float EditorManager::_Sensitivity = 0.1f;
float EditorManager::_LastX = 0;
float EditorManager::_LastY = 0;
float EditorManager::_LastScrollY = 0;
bool EditorManager::_StartMouse = false;
bool EditorManager::_StartScroll = false;
bool EditorManager::_LocalPositionSelected = true;
bool EditorManager::_LocalRotationSelected = false;
bool EditorManager::_LocalScaleSelected = false;
inline bool UniEngine::EditorManager::DrawEntityMenu(bool enabled, Entity& entity)
{
	bool deleted = false;
	if (ImGui::BeginPopupContextItem(std::to_string(entity.Index).c_str()))
	{
		if (ImGui::Button("Delete")) {
			EntityManager::DeleteEntity(entity);
			deleted = true;
		}
		if (!deleted && ImGui::Button(enabled ? "Disable" : "Enable")) {
			if (enabled) {
				entity.SetEnabled(false);
			}
			else {
				entity.SetEnabled(true);
			}
		}
		if (!deleted && ImGui::BeginMenu("Rename"))
		{
			static char newName[256];
			ImGui::InputText("New name", newName, 256);
			if (ImGui::Button("Confirm"))EntityManager::SetEntityName(entity, std::string(newName));

			ImGui::EndMenu();
		}
		ImGui::EndPopup();
	}
	return deleted;
}

void UniEngine::EditorManager::DrawEntityNode(Entity& entity)
{
	std::string title = std::to_string(entity.Index) + ": ";
	title += entity.GetName();
	bool enabled = entity.Enabled();
	if (enabled) {
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4({ 1, 1, 1, 1 }));
	}

	bool opened = ImGui::TreeNodeEx(title.c_str(), ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_NoAutoOpenOnLog | (_SelectedEntity == entity ? ImGuiTreeNodeFlags_Framed : ImGuiTreeNodeFlags_FramePadding));
	if (enabled) {
		ImGui::PopStyleColor();
	}
	if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(0)) {
		_SelectedEntity = entity;
	}
	const bool deleted = DrawEntityMenu(enabled, entity);
	if (opened && !deleted) {
		ImGui::TreePush();
		EntityManager::ForEachChild(entity, [](Entity child) {
			DrawEntityNode(child);
			});
		ImGui::TreePop();
	}
}

void UniEngine::EditorManager::InspectComponentData(ComponentBase* data, ComponentType type, bool isRoot)
{
	if (_ComponentDataInspectorMap.find(type.TypeID) != _ComponentDataInspectorMap.end()) {
		_ComponentDataInspectorMap.at(type.TypeID)(data, isRoot);
	}
}

void UniEngine::EditorManager::Init()
{
	_Enabled = true;
	_BasicEntityArchetype = EntityManager::CreateEntityArchetype("General", GlobalTransform(), Transform());

	RegisterComponentDataInspector<GlobalTransform>([](ComponentBase* data, bool isRoot)
		{
			std::stringstream stream;
			GlobalTransform* ltw = reinterpret_cast<GlobalTransform*>(data);
			glm::vec3 er;
			glm::vec3 t;
			glm::vec3 s;
			ltw->Decompose(t, er, s);
			er = glm::degrees(er);
			ImGui::DragFloat3("Position", &t.x, 1, 0, 0, "%.3f", ImGuiInputTextFlags_ReadOnly);
			ImGui::DragFloat3("Rotation", &er.x, 1, 0, 0, "%.3f", ImGuiInputTextFlags_ReadOnly);
			ImGui::DragFloat3("Scale", &s.x, 1, 0, 0, "%.3f", ImGuiInputTextFlags_ReadOnly);
		}
	);


	RegisterComponentDataInspector<Transform>([](ComponentBase* data, bool isRoot)
		{
			std::stringstream stream;
			auto ltp = static_cast<Transform*>(static_cast<void*>(data));
			bool edited = false;
			if (ltp != _PreviouslyStoredTransform) {
				_PreviouslyStoredTransform = ltp;
				ltp->Decompose(_PreviouslyStoredPosition, _PreviouslyStoredRotation, _PreviouslyStoredScale);
				_PreviouslyStoredRotation = glm::degrees(_PreviouslyStoredRotation);
			}
			if (ImGui::DragFloat3("##Local Position", &_PreviouslyStoredPosition.x, 0.1f)) edited = true;
			ImGui::SameLine();
			if (ImGui::Selectable("Local Position", &_LocalPositionSelected) && _LocalPositionSelected)
			{
				_LocalRotationSelected = false;
				_LocalScaleSelected = false;
			}
			if (ImGui::DragFloat3("##Local Rotation", &_PreviouslyStoredRotation.x, 1.0f)) edited = true;
			ImGui::SameLine();
			if (ImGui::Selectable("Local Rotation", &_LocalRotationSelected) && _LocalRotationSelected)
			{
				_LocalPositionSelected = false;
				_LocalScaleSelected = false;
			}
			if (ImGui::DragFloat3("##Local Scale", &_PreviouslyStoredScale.x, 0.01f)) edited = true;
			ImGui::SameLine();
			if (ImGui::Selectable("Local Scale", &_LocalScaleSelected) && _LocalScaleSelected)
			{
				_LocalRotationSelected = false;
				_LocalPositionSelected = false;
			}
			if (edited)
			{
				ltp->Value = glm::translate(_PreviouslyStoredPosition) * glm::mat4_cast(glm::quat(glm::radians(_PreviouslyStoredRotation))) * glm::scale(_PreviouslyStoredScale);
			}
		}
	);

	RegisterComponentDataInspector<DirectionalLight>([](ComponentBase* data, bool isRoot)
		{
			std::stringstream stream;
			stream << std::hex << "0x" << (size_t)data;
			auto* dl = static_cast<DirectionalLight*>((void*)data);
			ImGui::ColorEdit3("Diffuse", &dl->diffuse[0]);
			ImGui::DragFloat("Diffuse Brightness", &dl->diffuseBrightness, 0.1f, 0.0f);
			ImGui::ColorEdit3("Specular", &dl->specular[0]);
			ImGui::DragFloat("Specular Brightness", &dl->specularBrightness, 0.1f, 0.0f);
			ImGui::DragFloat("Bias", &dl->depthBias, 0.001f, 0.0f);
			ImGui::InputFloat("Normal Offset", &dl->normalOffset, 0.001f, 0.0f);
			ImGui::DragFloat("Light Size", &dl->lightSize, 0.01f, 0.0f);
		});
	RegisterComponentDataInspector<PointLight>([](ComponentBase* data, bool isRoot)
		{
			std::stringstream stream;
			stream << std::hex << "0x" << (size_t)data;
			auto* pl = static_cast<PointLight*>((void*)data);
			ImGui::ColorEdit3("Diffuse", &pl->diffuse[0]);
			ImGui::DragFloat("Diffuse Brightness", &pl->diffuseBrightness, 0.1f, 0.0f);
			ImGui::ColorEdit3("Specular", &pl->specular[0]);
			ImGui::DragFloat("Specular Brightness", &pl->specularBrightness, 0.1f, 0.0f);
			ImGui::DragFloat("Bias", &pl->bias, 0.001f, 0.0f);

			ImGui::DragFloat("Constant", &pl->constant, 0.01f, 0.0f);
			ImGui::DragFloat("Linear", &pl->linear, 0.0001f, 0, 1, "%.4f");
			ImGui::DragFloat("Quadratic", &pl->quadratic, 0.00001f, 0, 10, "%.5f");

			//ImGui::InputFloat("Normal Offset", &dl->normalOffset, 0.01f);
			ImGui::DragFloat("Light Size", &pl->lightSize, 0.01f);
		});

	RegisterComponentDataInspector<SpotLight>([](ComponentBase* data, bool isRoot)
		{
			std::stringstream stream;
			stream << std::hex << "0x" << (size_t)data;
			auto* sl = static_cast<SpotLight*>((void*)data);
			ImGui::ColorEdit3("Diffuse", &sl->diffuse[0]);
			ImGui::DragFloat("Diffuse Brightness", &sl->diffuseBrightness, 0.1f, 0.0f);
			ImGui::ColorEdit3("Specular", &sl->specular[0]);
			ImGui::DragFloat("Specular Brightness", &sl->specularBrightness, 0.1f, 0.0f);
			ImGui::DragFloat("Bias", &sl->bias, 0.001f, 0.0f);

			ImGui::DragFloat("Constant", &sl->constant, 0.01f, 0.0f);
			ImGui::DragFloat("Linear", &sl->linear, 0.001f, 0, 1, "%.3f");
			ImGui::DragFloat("Quadratic", &sl->quadratic, 0.001f, 0, 10, "%.4f");

			ImGui::DragFloat("Inner Degrees", &sl->innerDegrees, 0.1f, 0.0f, sl->outerDegrees);
			ImGui::DragFloat("Outer Degrees", &sl->outerDegrees, 0.1f, sl->innerDegrees, 180.0f);
			ImGui::DragFloat("Light Size", &sl->lightSize, 0.01f, 0.0f);
		});

	RegisterPrivateComponentMenu<CameraComponent>([](Entity owner)
		{
			if (owner.HasPrivateComponent<CameraComponent>()) return;
			if (ImGui::SmallButton("CameraComponent"))
			{
				owner.SetPrivateComponent(std::make_unique<CameraComponent>());
			}
		}
	);
	RegisterPrivateComponentMenu<RigidBody>([](Entity owner)
		{
			if (owner.HasPrivateComponent<RigidBody>()) return;
			if (Application::IsPlaying()) return;
			if (ImGui::SmallButton("RigidBody"))
			{
				owner.SetPrivateComponent(std::make_unique<RigidBody>());
			}
		}
	);
	RegisterPrivateComponentMenu<MeshRenderer>([](Entity owner)
		{
			if (owner.HasPrivateComponent<MeshRenderer>()) return;
			if (ImGui::SmallButton("MeshRenderer"))
			{
				owner.SetPrivateComponent(std::make_unique<MeshRenderer>());
			}
		}
	);
	RegisterPrivateComponentMenu<Particles>([](Entity owner)
		{
			if (owner.HasPrivateComponent<Particles>()) return;
			if (ImGui::SmallButton("Particles"))
			{
				owner.SetPrivateComponent(std::make_unique<Particles>());
			}
		}
	);

	RegisterComponentDataMenu<GlobalTransform>([](Entity owner)
		{
			if (owner.HasComponentData<GlobalTransform>()) return;
			if (ImGui::SmallButton("GlobalTransform"))
			{
				EntityManager::AddComponentData(owner, GlobalTransform());
			}
		}
	);

	RegisterComponentDataMenu<Transform>([](Entity owner)
		{
			if (owner.HasComponentData<Transform>()) return;
			if (ImGui::SmallButton("Transform"))
			{
				EntityManager::AddComponentData(owner, Transform());
			}
		}
	);

	RegisterComponentDataMenu<PointLight>([](Entity owner)
		{
			if (owner.HasComponentData<PointLight>()) return;
			if (ImGui::SmallButton("PointLight"))
			{
				EntityManager::AddComponentData(owner, PointLight());
			}
		}
	);

	RegisterComponentDataMenu<SpotLight>([](Entity owner)
		{
			if (owner.HasComponentData<SpotLight>()) return;
			if (ImGui::SmallButton("SpotLight"))
			{
				EntityManager::AddComponentData(owner, SpotLight());
			}
		}
	);

	RegisterComponentDataMenu<DirectionalLight>([](Entity owner)
		{
			if (owner.HasComponentData<DirectionalLight>()) return;
			if (ImGui::SmallButton("DirectionalLight"))
			{
				EntityManager::AddComponentData(owner, DirectionalLight());
			}
		}
	);

	_SelectedEntity.Index = 0;
	_ConfigFlags += EntityEditorSystem_EnableEntityHierarchy;
	_ConfigFlags += EntityEditorSystem_EnableEntityInspector;

	_SceneCameraPosition = glm::vec3(0.0f);
	_SceneCameraRotation = glm::quat(glm::vec3(0.0f));
	_SceneCamera = std::make_unique<CameraComponent>();
	_SceneCamera->DrawSkyBox = false;
}

void UniEngine::EditorManager::Destroy()
{

}
static const char* HierarchyDisplayMode[]{ "Archetype", "Hierarchy" };

void EditorManager::PreUpdate()
{
#pragma region ImGui
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	ImGuizmo::BeginFrame();
#pragma endregion
#pragma region Dock & Main Menu
	static bool opt_fullscreen_persistant = true;
	bool opt_fullscreen = opt_fullscreen_persistant;
	static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

	// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
	// because it would be confusing to have two docking targets within each others.
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;
	if (opt_fullscreen)
	{
		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->GetWorkPos());
		ImGui::SetNextWindowSize(viewport->GetWorkSize());
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
	}

	// When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
	// and handle the pass-thru hole, so we ask Begin() to not render a background.
	if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
		window_flags |= ImGuiWindowFlags_NoBackground;

	// Important: note that we proceed even if Begin() returns false (aka window is collapsed).
	// This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
	// all active windows docked into it will lose their parent and become undocked.
	// We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
	// any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	static bool openDock = true;
	ImGui::Begin("Root DockSpace", &openDock, window_flags);
	ImGui::PopStyleVar();
	if (opt_fullscreen)
		ImGui::PopStyleVar(2);
	ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
	ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
	ImGui::End();

	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::Button(Application::_Playing ? "Pause" : "Play"))
		{
			Application::_Playing = !Application::_Playing;
		}
		ImGui::Separator();
		if (ImGui::BeginMenu("File"))
		{
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Edit"))
		{
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("View"))
		{
			ImGui::Checkbox("Log", &_DisplayLog);
			ImGui::Checkbox("Error", &_DisplayError);
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Window"))
		{
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Help"))
		{
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}

#pragma endregion
	_SceneCamera->ResizeResolution(_SceneCameraResolutionX, _SceneCameraResolutionY);
	_SceneCamera->GetCamera()->Clear();

}


void UniEngine::EditorManager::Update()
{

}
void EditorManager::LateUpdate()
{
#pragma region Entity Explorer
	if (_ConfigFlags & EntityEditorSystem_EnableEntityHierarchy) {
		ImGui::Begin("Entity Explorer");
		if (ImGui::BeginPopupContextWindow("DataComponentInspectorPopup"))
		{
			if (ImGui::Button("Create new entity"))
			{
				auto newEntity = EntityManager::CreateEntity(_BasicEntityArchetype);
				newEntity.SetComponentData(Transform());
				newEntity.SetComponentData(GlobalTransform());
			}
			ImGui::EndPopup();
		}
		ImGui::Combo("Display mode", &_SelectedHierarchyDisplayMode, HierarchyDisplayMode, IM_ARRAYSIZE(HierarchyDisplayMode));
		if (_SelectedHierarchyDisplayMode == 0) {
			EntityManager::ForEachEntityStorageUnsafe([](int i, EntityComponentStorage storage) {
				ImGui::Separator();

				std::string title = std::to_string(i) + ". " + storage.ArchetypeInfo->Name;
				if (ImGui::CollapsingHeader(title.c_str())) {
					ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.2, 0.3, 0.2, 1.0));
					ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.2, 0.2, 0.2, 1.0));
					ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.2, 0.2, 0.3, 1.0));
					for (int j = 0; j < storage.ArchetypeInfo->EntityAliveCount; j++) {
						Entity entity = storage.ChunkArray->Entities.at(j);
						std::string title = std::to_string(entity.Index) + ": ";
						title += entity.GetName();
						bool enabled = entity.Enabled();
						if (enabled) {
							ImGui::PushStyleColor(ImGuiCol_Text, ImVec4({ 1, 1, 1, 1 }));
						}
						ImGui::TreeNodeEx(title.c_str(), ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoAutoOpenOnLog | (_SelectedEntity == entity ? ImGuiTreeNodeFlags_Framed : ImGuiTreeNodeFlags_FramePadding));
						if (enabled) {
							ImGui::PopStyleColor();
						}
						DrawEntityMenu(enabled, entity);
						if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(0)) {
							_SelectedEntity = entity;
						}
					}
					ImGui::PopStyleColor();
					ImGui::PopStyleColor();
					ImGui::PopStyleColor();
				}
				});
		}
		else if (_SelectedHierarchyDisplayMode == 1) {
			ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.2, 0.3, 0.2, 1.0));
			ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.2, 0.2, 0.2, 1.0));
			ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.2, 0.2, 0.3, 1.0));
			EntityManager::ForAllEntities([](int i, Entity entity) {
				if (EntityManager::GetParent(entity).IsNull()) DrawEntityNode(entity);
				});
			ImGui::PopStyleColor();
			ImGui::PopStyleColor();
			ImGui::PopStyleColor();
		}

		ImGui::End();
	}
#pragma endregion
#pragma region Entity Inspector
	if (_ConfigFlags & EntityEditorSystem_EnableEntityInspector) {
		ImGui::Begin("Entity Inspector");
		if (!_SelectedEntity.IsNull() && !_SelectedEntity.IsDeleted()) {
			std::string title = std::to_string(_SelectedEntity.Index) + ": ";
			title += _SelectedEntity.GetName();
			ImGui::Text(title.c_str());
			bool deleted = DrawEntityMenu(_SelectedEntity.Enabled(), _SelectedEntity);
			ImGui::Separator();
			if (!deleted) {
				if (ImGui::CollapsingHeader("Data components", ImGuiTreeNodeFlags_DefaultOpen)) {
					if (ImGui::BeginPopupContextItem("DataComponentInspectorPopup"))
					{
						ImGui::Text("Add data component: ");
						ImGui::Separator();
						for (auto& i : _ComponentDataMenuList)
						{
							i.second(_SelectedEntity);
						}
						ImGui::Separator();
						ImGui::EndPopup();
					}
					bool skip = false;
					int i = 0;
					EntityManager::ForEachComponentUnsafe(_SelectedEntity, [&skip, &i](ComponentType type, void* data)
						{
							if (skip) return;
							std::string info = std::string(type.Name + 7);
							info += " Size: " + std::to_string(type.Size);
							ImGui::Text(info.c_str());
							ImGui::PushID(i);
							if (ImGui::BeginPopupContextItem(("DataComponentDeletePopup" + std::to_string(i)).c_str()))
							{
								if (ImGui::Button("Remove"))
								{
									skip = true;
									EntityManager::RemoveComponentData(_SelectedEntity, type.TypeID);
								}
								ImGui::EndPopup();
							}
							ImGui::PopID();
							InspectComponentData(static_cast<ComponentBase*>(data), type, EntityManager::GetParent(_SelectedEntity).IsNull());
							ImGui::Separator();
							i++;
						}
					);
				}

				if (ImGui::CollapsingHeader("Private components", ImGuiTreeNodeFlags_DefaultOpen)) {
					if (ImGui::BeginPopupContextItem("PrivateComponentInspectorPopup"))
					{
						ImGui::Text("Add private component: ");
						ImGui::Separator();
						for (auto& i : _PrivateComponentMenuList)
						{
							i.second(_SelectedEntity);
						}
						ImGui::Separator();
						ImGui::EndPopup();
					}

					int i = 0;
					bool skip = false;
					EntityManager::ForEachPrivateComponent(_SelectedEntity, [&i, &skip](PrivateComponentElement& data)
						{
							if (skip) return;
							ImGui::Checkbox((data.Name + 6), &data.PrivateComponentData->_Enabled);
							ImGui::PushID(i);
							if (ImGui::BeginPopupContextItem(("PrivateComponentDeletePopup" + std::to_string(i)).c_str()))
							{
								if (ImGui::Button("Remove"))
								{
									skip = true;
									EntityManager::RemovePrivateComponent(_SelectedEntity, data.TypeID);
								}
								ImGui::EndPopup();
							}
							ImGui::PopID();
							if (!skip) {
								if (ImGui::TreeNode(("Component Settings##" + std::to_string(i)).c_str())) {
									data.PrivateComponentData->OnGui();
									ImGui::TreePop();
								}
								ImGui::Separator();
								i++;
							}
						}
					);
				}

				if (ImGui::CollapsingHeader("Shared components", ImGuiTreeNodeFlags_DefaultOpen)) {
					int i = 0;
					EntityManager::ForEachSharedComponent(_SelectedEntity, [&i](SharedComponentElement data)
						{
							ImGui::Checkbox((data.Name + 6), &data.SharedComponentData->_Enabled);
							if (ImGui::TreeNode(("Component Settings##" + std::to_string(i)).c_str())) {
								data.SharedComponentData->OnGui();
								ImGui::TreePop();
							}
							ImGui::Separator();
							i++;
						});
				}

			}
		}
		else {
			_SelectedEntity.Index = 0;
		}
		ImGui::End();
	}
#pragma endregion
#pragma region Scene Window
	ImVec2 viewPortSize;
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
	ImGui::Begin("Scene");
	{
		// Using a Child allow to fill all the space of the window.
		// It also allows customization
		if (ImGui::BeginChild("CameraRenderer")) {
			if (ImGui::IsWindowFocused())
			{
#pragma region Scene Camera Controller
				glm::vec3 front = _SceneCameraRotation * glm::vec3(0, 0, -1);
				glm::vec3 right = _SceneCameraRotation * glm::vec3(1, 0, 0);
				if (InputManager::GetKey(GLFW_KEY_W)) {
					_SceneCameraPosition += glm::vec3(front.x, 0.0f, front.z) * (float)Application::GetWorld()->Time()->DeltaTime() * _Velocity;
				}
				if (InputManager::GetKey(GLFW_KEY_S)) {
					_SceneCameraPosition -= glm::vec3(front.x, 0.0f, front.z) * (float)Application::GetWorld()->Time()->DeltaTime() * _Velocity;
				}
				if (InputManager::GetKey(GLFW_KEY_A)) {
					_SceneCameraPosition -= glm::vec3(right.x, 0.0f, right.z) * (float)Application::GetWorld()->Time()->DeltaTime() * _Velocity;
				}
				if (InputManager::GetKey(GLFW_KEY_D)) {
					_SceneCameraPosition += glm::vec3(right.x, 0.0f, right.z) * (float)Application::GetWorld()->Time()->DeltaTime() * _Velocity;
				}
				if (InputManager::GetKey(GLFW_KEY_LEFT_SHIFT)) {
					_SceneCameraPosition.y += _Velocity * (float)Application::GetWorld()->Time()->DeltaTime();
				}
				if (InputManager::GetKey(GLFW_KEY_LEFT_CONTROL)) {
					_SceneCameraPosition.y -= _Velocity * (float)Application::GetWorld()->Time()->DeltaTime();
				}
				auto mousePosition = InputManager::GetMouseAbsolutePosition();
				if (!_StartMouse) {
					_LastX = mousePosition.x;
					_LastY = mousePosition.y;
					_StartMouse = true;
				}
				float xoffset = mousePosition.x - _LastX;
				float yoffset = -mousePosition.y + _LastY;
				_LastX = mousePosition.x;
				_LastY = mousePosition.y;
				if (InputManager::GetMouse(GLFW_MOUSE_BUTTON_RIGHT)) {
					if (xoffset != 0 || yoffset != 0) {
						_SceneCameraRotation = _SceneCamera.get()->GetCamera()->ProcessMouseMovement(xoffset, yoffset, _Sensitivity);
					}
				}
#pragma endregion
			}
			viewPortSize = ImGui::GetWindowSize();
			// Because I use the texture from OpenGL, I need to invert the V from the UV.
			ImGui::Image((ImTextureID)_SceneCamera->GetCamera()->GetTexture()->ID(), viewPortSize, ImVec2(0, 1), ImVec2(1, 0));
			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_MODEL"))
				{
					IM_ASSERT(payload->DataSize == sizeof(std::shared_ptr<Model>));
					std::shared_ptr<Model> payload_n = *(std::shared_ptr<Model>*)payload->Data;
					EntityArchetype archetype = EntityManager::CreateEntityArchetype("Model", Transform(), GlobalTransform());
					GlobalTransform ltw;
					AssetManager::ToEntity(archetype, payload_n).SetComponentData(ltw);
				}
				ImGui::EndDragDropTarget();
			}
#pragma region Gizmos
			if (!_SelectedEntity.IsNull() && !_SelectedEntity.IsDeleted() && (_LocalPositionSelected || _LocalRotationSelected || _LocalScaleSelected))
			{
				ImGuizmo::SetOrthographic(false);
				ImGuizmo::SetDrawlist();
				ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, viewPortSize.x, viewPortSize.y);
				Transform transform = _SelectedEntity.GetComponentData<Transform>();
				glm::mat4 cameraView = glm::inverse(glm::translate(_SceneCameraPosition) * glm::mat4_cast(_SceneCameraRotation));
				glm::mat4 cameraProjection = _SceneCamera->GetCamera()->GetProjection();
				auto op = _LocalPositionSelected ? ImGuizmo::OPERATION::TRANSLATE : _LocalRotationSelected ? ImGuizmo::OPERATION::ROTATE : ImGuizmo::OPERATION::SCALE;
				ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection), op, ImGuizmo::LOCAL, glm::value_ptr(transform.Value));
				if (ImGuizmo::IsUsing()) {
					_SelectedEntity.SetComponentData(transform);
					transform.Decompose(_PreviouslyStoredPosition, _PreviouslyStoredRotation, _PreviouslyStoredScale);
				}
			}
#pragma endregion

		}
		ImGui::EndChild();
		_SceneCamera->SetEnabled(!(ImGui::GetCurrentWindowRead()->Hidden && !ImGui::GetCurrentWindowRead()->Collapsed));
	}
	ImGui::End();
	ImGui::PopStyleVar();
	_SceneCameraResolutionX = viewPortSize.x;
	_SceneCameraResolutionY = viewPortSize.y;

#pragma endregion
#pragma region Logs and errors
	if (_DisplayLog) {
		ImGui::Begin("Log");
		size_t size = Debug::GetLogs()->size();
		std::string logs = "";
		for (int i = (int)size - 1; i >= 0; i--) {
			logs += Debug::GetLogs()->at(i)->c_str();
		}
		ImGui::Text(logs.c_str());
		ImGui::End();
	}
	if (_DisplayError)
	{
		ImGui::Begin("Error");
		size_t size = Debug::GetErrors()->size();
		std::string logs = "";
		for (int i = (int)size - 1; i >= 0; i--) {
			logs += Debug::GetErrors()->at(i)->c_str();
		}
		ImGui::Text(logs.c_str());
		ImGui::End();
	}
#pragma endregion
#pragma region ImGui
	RenderTarget::BindDefault();
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	// Update and Render additional Platform Windows
	// (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
	//  For this specific demo app we could also call glfwMakeContextCurrent(window) directly)
	if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		GLFWwindow* backup_current_context = glfwGetCurrentContext();
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
		glfwMakeContextCurrent(backup_current_context);
	}
#pragma endregion
}



void UniEngine::EditorManager::SetSelectedEntity(Entity entity)
{
	if (entity.IsNull() || entity.IsDeleted()) return;
	_SelectedEntity = entity;
}
