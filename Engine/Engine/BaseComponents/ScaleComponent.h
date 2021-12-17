#pragma once

#include "../Engine.h"
#include "../glm_printer.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <nlohmann/json.hpp>

namespace diffusion {

	struct ScaleComponent
	{
		glm::vec3 Scale{ 1.0f, 1.0f, 1.0f };

		NLOHMANN_DEFINE_TYPE_INTRUSIVE(ScaleComponent, Scale)
	};

}