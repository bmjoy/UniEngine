#include "pch.h"
#include "ComponentFactory.h"
#include "Debug.h"
using namespace UniEngine;

bool ComponentFactory::RegisterComponentData(const std::string& id,
	const std::function<std::shared_ptr<ComponentBase>(size_t&, size_t&)>& func)
{
	return Get()._ComponentDataGenerators.insert({ id, func }).second;
}

std::shared_ptr<ComponentBase> ComponentFactory::ProduceComponentData(const std::string& id, size_t& hashCode, size_t& size)
{
	auto& factory = Get();
	const auto it = factory._ComponentDataGenerators.find(id);
	if(it != Get()._ComponentDataGenerators.end())
	{
		return it->second(hashCode, size);
	}
	Debug::Error("Component " + id + "is not registered!");
	throw 1;
}

bool ComponentFactory::RegisterComponent(const std::string& id,
	const std::function<Serializable* ()>& func)
{
	return Get()._ClassComponentGenerators.insert({ id, func }).second;
}

Serializable* ComponentFactory::ProduceSharedComponent(const std::string& id)
{
	const auto it = Get()._ClassComponentGenerators.find(id);
	if (it != Get()._ClassComponentGenerators.end())
	{
		return it->second();
	}
	Debug::Error("Component " + id + "is not registered!");
	throw 1;
}
