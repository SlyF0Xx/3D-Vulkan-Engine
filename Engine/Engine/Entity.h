#pragma once

#include "ComponentGuard.h"

#include <vector>

namespace diffusion {

class Entity
{
public:
	Entity(std::vector<ComponentGuard> components)
		: m_components(std::move(components))
	{}

	std::vector<std::reference_wrapper<Component>> get_components();
	Component& add_component(ComponentGuard component);

	template<typename COMPONENT_TYPE>
	COMPONENT_TYPE* get_first_component_by_tag(Component::Tag tag);

private:
	std::vector<ComponentGuard> m_components;
};

template<typename COMPONENT_TYPE>
COMPONENT_TYPE* Entity::get_first_component_by_tag(Component::Tag tag)
{
	for (auto& inner_component : get_components()) {
		auto it = std::find(
			inner_component.get().get_tags().begin(),
			inner_component.get().get_tags().end(),
			tag);
		if (it != inner_component.get().get_tags().end()) {
			return dynamic_cast<COMPONENT_TYPE*>(&inner_component.get());
		}
	}
	return nullptr;
}

} // namespace diffusion 