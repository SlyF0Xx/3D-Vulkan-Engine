#include "Identifier.h"

namespace diffusion {

static uint64_t s_id = 0;

Identifier::Identifier() :
	m_id(s_id++)
{}

} // namespace diffusion {