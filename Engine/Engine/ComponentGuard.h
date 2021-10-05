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

	ComponentGuard(const ComponentGuard&) = delete;
	/*
	ComponentGuard(const ComponentGuard&) {
		throw std::exception("kek");
	}
	*/
	ComponentGuard(ComponentGuard&& other) noexcept
		: m_id(other.m_id)
	{
		other.m_id.reset();
	}

	ComponentGuard& operator= (ComponentGuard&& other) noexcept
	{
		m_id = other.m_id;
		other.m_id.reset();
	}

	~ComponentGuard()
	{
		s_component_manager_instance.destroy_component(m_id);
	}

	Component& get_component()
	{
		return s_component_manager_instance.get_component_by_id(m_id);
	}

private:
	Component::ComponentIdentifier m_id;
};

} // namespace diffusion {