#pragma once

#include "Component.h"

#include <vector>
#include <memory>
#include <map>
#include <set>

namespace diffusion {

class ComponentManager
{
public:
	void create_component(std::unique_ptr<Component> component);
	void destroy_component(Component::ComponentIdentifier id);
	std::vector<std::reference_wrapper<Component>> get_components_by_tag(const Component::Tag& tag);
	std::vector<std::reference_wrapper<Component>> get_components_by_tags(const std::vector<Component::Tag>& tags);

private:
	void register_component(const Component& component);
	void unregister_component(const Component& component);

	std::multimap<Component::Tag, Component::ComponentIdentifier> components_by_tag;
	std::map<Component::ComponentIdentifier, std::unique_ptr<Component>> m_components;
};

ComponentManager s_component_manager_instance;

} // namespace diffusion {