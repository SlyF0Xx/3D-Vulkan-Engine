#pragma once

#include "ComponentGuard.h"

#include <vector>

namespace diffusion {

class Entity
{
public:
	Entity(const std::vector<ComponentGuard>& components)
		: m_components(components)
	{}

private:
	std::vector<ComponentGuard> m_components;
};

} // namespace diffusion 