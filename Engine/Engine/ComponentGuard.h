#pragma once

#include "Component.h"
#include "ComponentManager.h"

#include <memory>

namespace diffusion {

class ComponentGuard
{
public:
	ComponentGuard(std::unique_ptr<Component> component)
		: m_id(component->get_id())
	{
		s_component_manager_instance.create_component(std::move(component));
	}

	~ComponentGuard()
	{
		s_component_manager_instance.destroy_component(m_id);
	}

private:
	Component::ComponentIdentifier m_id;
};

} // namespace diffusion {