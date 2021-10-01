#pragma once

#include <cstdint>
#include <compare>

namespace diffusion {

class Identifier
{
public:
	Identifier();
	auto operator<=>(const Identifier&) const = default;

private:
	uint64_t m_id;
};

} // namespace diffusion {