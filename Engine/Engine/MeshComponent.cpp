#include "MeshComponent.h"

namespace diffusion {

MeshComponent::MeshComponent(
	Game& game,
	const std::vector<PrimitiveColoredVertex>& verticies,
	const std::vector<uint32_t>& indexes,
	const std::vector<Tag>& tags,
	Entity* parent)
	: Component(concat_vectors({ s_mesh_component_tag }, tags), parent), m_game(game), m_verticies(verticies), m_indexes(indexes)
{
}

} // namespace diffusion {