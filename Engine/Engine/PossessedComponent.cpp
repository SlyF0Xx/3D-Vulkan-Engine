#include "PossessedComponent.h"

namespace diffusion {

PossessedComponent::PossessedComponent(const std::vector<Tag>& tags, Entity* parent)
	: Component(concat_vectors({ s_possessed_tag }, tags), parent)
{
}

} // namespace diffusion {