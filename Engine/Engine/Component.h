#pragma once

#include "Identifier.h"

#include <vector>

namespace diffusion {

class Component
{
public:
	using ComponentIdentifier = Identifier;
	using Tag = Identifier;

	Component(const std::vector<Tag>& tags)
		: m_tags(tags)
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

private:
	std::vector<Tag> m_tags;
	ComponentIdentifier m_id;
};

} // namespace diffusion {
