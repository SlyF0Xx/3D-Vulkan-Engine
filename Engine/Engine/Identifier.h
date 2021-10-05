#pragma once

#include <cstdint>
#include <compare>

namespace diffusion {

class Identifier
{
public:
	Identifier();
	auto operator<=>(const Identifier&) const = default;
	void reset()
	{
		m_id = -1;
	}

private:
	uint64_t m_id;
};

} // namespace diffusion {