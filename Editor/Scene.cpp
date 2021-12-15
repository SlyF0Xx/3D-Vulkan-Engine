#include "Scene.h"

Editor::Scene::Scene(const diffusion::Ref<Game>& game, uint32_t id) {
    m_Context = game;
    m_ID = id;
}

Editor::Scene Editor::Scene::GetEmpty(const diffusion::Ref<Game>& game, uint32_t id) {
    Scene scene = Scene(game, id);
    auto mainEntity = scene.m_Context->get_registry().create();
    scene.m_Context->get_registry().set<diffusion::PossessedEntity>(mainEntity);
    scene.m_Context->get_registry().emplace<diffusion::TagComponent>(mainEntity, "Main Entity");

    auto mainCamera = scene.m_Context->get_registry().create();
    scene.m_Context->get_registry().set<diffusion::MainCameraTag>(mainCamera);
    scene.m_Context->get_registry().emplace<diffusion::TagComponent>(mainCamera, "Main Camera");

    auto directionalLight = diffusion::create_point_light_entity(
        scene.m_Context->get_registry(), 
        glm::vec3(0, 0, 4)
    );
    scene.m_Context->get_registry().emplace<diffusion::TagComponent>(directionalLight, "Point Light");

    scene.CreateEditorCamera();
    return scene;
}

const std::string Editor::Scene::GetTitle() const {
    return m_Title;
}

const uint32_t Editor::Scene::GetID() const {
    return m_ID;
}

void Editor::Scene::CreateEditorCamera() {
    // TODO: Hide it from scene hierarchy.
    m_EditorCamera = m_Context->get_registry().create();
    m_Context->get_registry().emplace<diffusion::TransformComponent>(m_EditorCamera, diffusion::create_matrix());
    m_Context->get_registry().emplace<diffusion::CameraComponent>(m_EditorCamera);
    m_Context->get_registry().emplace<diffusion::debug_tag>(m_EditorCamera);
    m_Context->get_registry().set<diffusion::MainCameraTag>(m_EditorCamera);
    m_Context->get_registry().emplace<diffusion::TagComponent>(m_EditorCamera, "Editor Camera");
}
