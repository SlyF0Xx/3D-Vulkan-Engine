#include "Relation.h"

namespace diffusion {

void destroy_entity(entt::registry& registry, entt::entity& entity)
{
	auto* childs = registry.try_get<Childs>(entity);
	if (childs) {
		for (auto child : childs->m_childs) {
			destroy_entity(registry, child);
		}
	}

	auto* parent = registry.try_get<Relation>(entity);
	if (parent) {
		auto parent_childs = registry.get<Childs>(parent->m_parent);
		parent_childs.m_childs.erase(entity);
	}

	registry.destroy(entity);
}

void unbind_entity(entt::registry& registry, entt::entity& entity)
{
	auto* parent = registry.try_get<Relation>(entity);
	if (parent) {
		registry.patch<Childs>(parent->m_parent, [&entity](Childs& childs) {
			childs.m_childs.erase(entity);
		});
	}

	registry.erase<Relation>(entity);
}

void rebind_entity(entt::registry& registry, entt::entity& entity, entt::entity& new_parent)
{
	auto* parent = registry.try_get<Relation>(entity);
	if (parent) {
		registry.patch<Childs>(parent->m_parent, [&entity](Childs & childs) {
			childs.m_childs.erase(entity);
		});
	}

	registry.erase<Relation>(entity);
	registry.emplace<Relation>(entity, new_parent);
}

} // namespace diffusion