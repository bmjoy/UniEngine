#include "pch.h"
#include "Entity.h"

#include <utility>
#include "EntityManager.h"
using namespace UniEngine;
inline bool UniEngine::Entity::Enabled() const
{
	return EntityManager::IsEntityEnabled(*this);
}

inline void UniEngine::Entity::SetEnabled(bool value) const
{
	EntityManager::SetEnable(*this, value);
}

void Entity::SetEnabledSingle(bool value) const
{
	EntityManager::SetEnableSingle(*this, value);
}

bool Entity::IsNull() const
{
	return Index == 0;
}

bool UniEngine::Entity::IsDeleted() const
{
	return EntityManager::IsEntityDeleted(Index);
}

bool Entity::IsValid() const
{
	if(!IsNull() && EntityManager::IsEntityValid(*this)) return true;
	return false;
}

inline std::string Entity::GetName() const
{
	return EntityManager::GetEntityName(*this);
}

inline void Entity::SetName(std::string name) const
{
	return EntityManager::SetEntityName(*this, std::move(name));
}

std::string EntityArchetype::GetName() const
{
	return EntityManager::GetEntityArchetypeName(*this);
}

bool EntityArchetypeInfo::HasType(size_t typeID)
{
	for (const auto& type : ComponentTypes)
	{
		if (typeID == type.TypeID) return true;
	}
	return false;
}
