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

private:
	std::vector<ComponentGuard> m_components;
};

} // namespace diffusion 