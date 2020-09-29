#pragma once
#include "EntitiesAPI.h"
namespace UniEngine {
#pragma region EntityManager
#pragma region Entity
	struct ENTITIES_API ComponentType {
		const char* Name;
		size_t TypeID;
		//Size of component
		size_t Size;
		//Starting point of the component, this part is not comparable
		size_t Offset;
		bool operator ==(const ComponentType& other) const {
			return (other.TypeID == TypeID) && (other.Size == Size);
		}
		bool operator !=(const ComponentType& other) const {
			return (other.TypeID != TypeID) || (other.Size != Size);
		}
	};

	struct ENTITIES_API ComponentBase {
	};


	class ENTITIES_API SharedComponentBase {
		
	public:
		virtual size_t GetHashCode() = 0;
		virtual void OnGui() {}
	};

	struct ENTITIES_API Entity {
		//Position in _Entity Array
		unsigned Index = 0;
		unsigned Version = 0;

		Entity() {
			Index = 0;
			Version = 0;
		}

		bool operator ==(const Entity& other) const {
			return (other.Index == Index) && (other.Version == Version);
		}
		bool operator !=(const Entity& other) const {
			return (other.Index != Index) || (other.Version != Version);
		}
		size_t operator() (Entity const& key) const
		{
			return (size_t)Index;
		}

		bool Enabled();

		void SetEnabled(bool value);

		bool IsNull() {
			return Index == 0;
		}

		bool IsDeleted();

		template<typename T = ComponentBase>
		void SetComponentData(T value);
		template<typename T = ComponentBase>
		T GetComponentData();
		template<typename T = ComponentBase>
		bool HasComponentData();
		template <typename T = SharedComponentBase>
		std::shared_ptr<T> GetSharedComponent();
		template <typename T = SharedComponentBase>
		void SetSharedComponent(T* value);
		template <typename T = SharedComponentBase>
		bool RemoveSharedComponent();
		template <typename T = SharedComponentBase>
		bool HasSharedComponent();

		inline std::string GetName();
		inline bool SetName(std::string name);
	};
#pragma region Storage



	template<typename T>
	ComponentType typeof() {
		ComponentType type;
		type.Name = typeid(T).name();
		type.Size = sizeof(T);
		type.Offset = 0;
		type.TypeID = typeid(T).hash_code();
		return type;
	}

	const size_t ARCHETYPECHUNK_SIZE = 16384;

	struct ComponentDataChunk {
		//16k raw data
		void* Data;
		template<typename T>
		T GetData(size_t offset);
		template<typename T>
		void SetData(size_t offset, T data);
		void ClearData(size_t offset, size_t size) {
			memset((void*)((char*)Data + offset), 0, size);
		}
	};

	template<typename T>
	inline T ComponentDataChunk::GetData(size_t offset)
	{
		return T(*(T*)((char*)Data + offset));
	}
	template<typename T>
	inline void ComponentDataChunk::SetData(size_t offset, T data)
	{
		*(T*)((char*)Data + offset) = data;
	}


	struct ComponentDataChunkArray {
		std::vector<Entity> Entities;
		std::vector<ComponentDataChunk> Chunks;
	};

	struct ENTITIES_API EntityArchetype {
		size_t Index;
		bool IsNull() {
			return Index == 0;
		}
	};

	struct SharedComponentElement
	{
		const char* Name;
		size_t TypeID;
		std::shared_ptr<SharedComponentBase> SharedComponentData;
		SharedComponentElement(const char* name, size_t id, std::shared_ptr<SharedComponentBase> data)
		{
			Name = name;
			TypeID = id;
			SharedComponentData = std::move(data);
		}
	};
	
	struct EntityInfo {
		std::string Name;
		size_t Version;
		bool Enabled = true;
		Entity Parent;
		std::vector<SharedComponentElement> SharedComponentElements;
		std::vector<Entity> Children;
		size_t ArchetypeInfoIndex;
		size_t ChunkArrayIndex;
	};

	struct ENTITIES_API EntityArchetypeInfo {
		std::string Name;
		size_t Index;
		//std::map<size_t, ComponentType> ComponentTypes;
		std::vector<ComponentType> ComponentTypes;
		//The size of a single entity
		size_t EntitySize;
		//How many entities can fit into one chunk
		size_t ChunkCapacity;
		//Current allocated entity count.
		size_t EntityCount = 0;
		//Current live entity count.
		size_t EntityAliveCount = 0;
		template<typename T>
		bool HasType();
		bool HasType(size_t typeID);
	};

	struct ENTITIES_API EntityQuery {
		size_t Index = 0;
		size_t Version = 0;

		bool operator ==(const EntityQuery& other) const {
			return (other.Index == Index) && (other.Version == Version);
		}
		bool operator !=(const EntityQuery& other) const {
			return (other.Index != Index) || (other.Version != Version);
		}
		size_t operator() (EntityQuery const& key) const
		{
			return Index;
		}

		bool IsNull() const
		{
			return Index == 0;
		}

		bool IsDeleted() const
		{
			return Version == 0;
		}
		template<typename T1 = ComponentBase>
		void ToComponentDataArray(std::vector<T1>& container);
		template<typename T1 = ComponentBase, typename T2 = ComponentBase>
		void ToComponentDataArray(std::vector<T1>& container, const std::function<bool(T2&)>& filterFunc);
		template<typename T1 = ComponentBase, typename T2 = ComponentBase, typename T3 = ComponentBase>
		void ToComponentDataArray(std::vector<T1>& container, const std::function<bool(T2&, T3&)>& filterFunc);
		template<typename T1 = ComponentBase, typename T2 = ComponentBase>
		void ToComponentDataArray(T1 filter, std::vector<T2>& container);
		void ToEntityArray(std::vector<Entity>& container);
		template<typename T1 = ComponentBase>
		void ToEntityArray(T1 filter, std::vector<Entity>& container);
		template<typename T1 = ComponentBase>
		void ToEntityArray(std::vector<Entity>& container, const std::function<bool(Entity, T1&)>& filterFunc);
		size_t GetEntityAmount();
	};



	struct ENTITIES_API EntityComponentStorage {
		EntityArchetypeInfo* ArchetypeInfo;
		ComponentDataChunkArray* ChunkArray;
		EntityComponentStorage(EntityArchetypeInfo* info, ComponentDataChunkArray* array);
	};

	struct EntityQueryInfo {
		std::vector<ComponentType> AllComponentTypes;
		std::vector<ComponentType> AnyComponentTypes;
		std::vector<ComponentType> NoneComponentTypes;
		std::vector<EntityComponentStorage> QueriedStorages;
	};
#pragma endregion
#pragma endregion

#pragma endregion
	template<typename T>
	inline bool EntityArchetypeInfo::HasType()
	{
		for (auto i : ComponentTypes) {
			if (i.TypeID == typeid(T).hash_code()) return true;
		}
		return false;
	}
}