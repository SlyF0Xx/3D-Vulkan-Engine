#include "RotateSystem.h"

#include "ImportableEntity.h"

namespace diffusion {

RotateSystem::RotateSystem()
	: System({ CatImportableEntity::s_special_cat_transform_tag })
{
}

} // namespace diffusion {