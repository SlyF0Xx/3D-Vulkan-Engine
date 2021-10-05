#pragma once

#include "Component.h"
#include "ComponentManager.h"

#include <vector>

namespace diffusion {

class System
{
public:
	System(std::vector<Component::Tag> tags)
		: m_tags(tags)
	{}

	virtual void components_callback(const std::vector<std::reference_wrapper<Component>>& components);
	virtual void component_callback(const std::reference_wrapper<Component>& component) {}
	virtual void tick();

private:
	std::vector<Component::Tag> m_tags;
};

} // namespace diffusion {