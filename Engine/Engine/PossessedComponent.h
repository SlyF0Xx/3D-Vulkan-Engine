#pragma once
#include "Component.h"

namespace diffusion {

class PossessedComponent :
    public Component
{
public:
	PossessedComponent(const std::vector<Tag>& tags, Entity* parent);

	static inline Component::Tag s_possessed_tag;
};

} // namespace diffusion {