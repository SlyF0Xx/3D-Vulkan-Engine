#pragma once

#include <entt/entt.hpp>
#include <nlohmann/json.hpp>

namespace diffusion {

struct TagComponent {
	std::string m_Tag;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(TagComponent, m_Tag)
};

}
