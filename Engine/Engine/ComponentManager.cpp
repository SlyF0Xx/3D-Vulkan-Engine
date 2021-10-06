#include "ComponentManager.h"

#include <algorithm>

namespace diffusion {

void ComponentManager::create_component(std::unique_ptr<Component> component)
{
	auto inserted_component_pair = m_components.emplace(component->get_id(), std::move(component));
	register_component(*inserted_component_pair.first->second);
}

void ComponentManager::destroy_component(Component::ComponentIdentifier id)
{
	auto it = m_components.find(id);
	if (it != m_components.end()) {
		unregister_component(*it->second);
		m_components.erase(it);
	}
}

std::vector<std::reference_wrapper<Component>> ComponentManager::get_components_by_tag(const Component::Tag& tag)
{
	std::vector<std::reference_wrapper<Component>> components;
	for (auto it = components_by_tag.find(tag); it != components_by_tag.end() && it->first == tag; ++it) {
		components.push_back(std::ref(*m_components.find(it->second)->second));
	}
	return components;
}

std::vector<std::reference_wrapper<Component>> ComponentManager::get_components_by_tags(const std::vector<Component::Tag>& tags)
{
	std::vector<std::reference_wrapper<Component>> components;
	for (auto& [_, component] : m_components) {
		if (std::all_of(tags.begin(), tags.end(), [&component] (const Component::Tag & tag) {
				return std::find(component->get_tags().begin(), component->get_tags().end(), tag) != component->get_tags().end();
			})) {
			components.push_back(std::ref(*component));
		}
	}

	/*
	std::set<Component::ComponentIdentifier> component_ids;
	for (auto& tag : tags) {
		for (auto it = components_by_tag.find(tag); it != components_by_tag.end() && it->first == tag; ++it) {
			component_ids.insert(it->second);
		}
	}

	std::vector<std::reference_wrapper<Component>> components;
	for (auto& component_id : component_ids) {
		components.push_back(std::ref(*m_components.find(component_id)->second));
	}
	*/
	return components;
}

Component& ComponentManager::get_component_by_id(Component::ComponentIdentifier id)
{
	return *m_components.find(id)->second;
}

void ComponentManager::register_component(const Component& component)
{
	for (auto& tag : component.get_tags()) {
		components_by_tag.insert(std::make_pair(tag, component.get_id()));
	}
}

void ComponentManager::unregister_component(const Component& component)
{
	for (auto& tag : component.get_tags()) {
		for (auto it = components_by_tag.find(tag); it != components_by_tag.end() && it->first == tag; ++it) {
			if (it->second == component.get_id()) {
				components_by_tag.erase(it);
				break;
			}
		}
	}
}

} // namespace diffusion {