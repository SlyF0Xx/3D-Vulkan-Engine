#include <Engine.h>

#include "MainLayout.h"
#include "MainWindow.h"

#include "Entities/ImportableEntity.h"
#include "Entities/CubeEntity.h"
#include "Entities/PlaneEntity.h"
#include "BaseComponents/CameraComponent.h"
#include "BaseComponents/PossessedComponent.h"
#include "Archiver.h"
#include "BaseComponents/Relation.h"
#include "BaseComponents/LitMaterial.h"
#include "BaseComponents/UnlitMaterial.h"
#include "BaseComponents/MeshComponent.h"
#include "Entities/DirectionalLightEntity.h"
#include "BaseComponents/DirectionalLightComponent.h"
#include "BaseComponents/PointLightComponent.h"
#include "BaseComponents/TagComponent.h"
#include "BaseComponents/DebugComponent.h"
#include "Entities/DebugCube.h"

#include <Engine.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include <chrono>
#include <iostream>
#include <map>
#include <fstream>

int main() {
	diffusion::Ref<Game> vulkan = diffusion::CreateRef<Game>();

	diffusion::Ref<Editor::EditorLayout> layout =
		diffusion::CreateRef<Editor::MainLayout>(vulkan);

	diffusion::Ref <Editor::EditorWindow> container = 
		diffusion::CreateRef<Editor::MainWindow>(layout);

	if (!container->Create()) {
		return 1;
	}

	vulkan->load_scene();

	container->StartMainLoop();

	return 0;
}