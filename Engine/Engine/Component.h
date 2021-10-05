#pragma once

#include "Identifier.h"

#include <vector>

namespace diffusion {

class Entity;

class Component
{
public:
	using ComponentIdentifier = Identifier;
	using Tag = Identifier;

	Component(const std::vector<Tag>& tags, Entity* parent)
		: m_tags(tags), m_parent(parent)
	{}

	virtual ~Component() = default;

	const std::vector<Tag>& get_tags() const
	{
		return m_tags;
	}

	ComponentIdentifier get_id()
	{
		return m_id;
	}

	ComponentIdentifier get_id() const
	{
		return m_id;
	}

	Entity* get_parrent()
	{
		return m_parent;
	}

	Entity* get_parrent() const
	{
		return m_parent;
	}

private:
	std::vector<Tag> m_tags;
	ComponentIdentifier m_id;
	Entity* m_parent;
};

inline std::vector<Component::Tag> concat_vectors(const std::vector<Component::Tag> & left, const std::vector<Component::Tag> & right)
{
	std::vector<Component::Tag> tmp = left;
	tmp.insert(tmp.end(), right.begin(), right.end());
	return tmp;
}

} // namespace diffusion {
