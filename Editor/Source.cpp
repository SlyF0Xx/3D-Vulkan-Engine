#include <Engine.h>

#include "MainLayout.h"
#include "MainWindow.h"
#include "GameProject.h"

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

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include <chrono>
#include <iostream>
#include <map>
#include <fstream>

int main() {
	// TODO: Scene должна создавать свой собственный контекст, поэтому передавать его нельзя.
	// TODO: GameProject должен получать контекст от текущей сцены.
	// diffusion::Ref<Game> vulkan = diffusion::CreateRef<Game>();

	Editor::GameProject::Instance()->CreateContext();

	diffusion::Ref<Editor::EditorWindow> mainWindow =
		diffusion::CreateRef<Editor::MainWindow>();

	diffusion::Ref<Editor::EditorLayout> layout =
		diffusion::CreateRef<Editor::MainLayout>();

	mainWindow->SetLayout(layout);

	if (!mainWindow->Create()) {
		return 1;
	}

	// TODO: Replace for reading .ini and load latest project.
	Editor::GameProject::Instance()->CreateEmpty();

	mainWindow->StartMainLoop();

	return 0;
}