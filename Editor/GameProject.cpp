#include "GameProject.h"

Editor::GameProject::GameProject(const diffusion::Ref<Game>& game) {
    m_Context = game;
}

bool Editor::GameProject::IsReady() const {
    return m_IsReady;
}

void Editor::GameProject::CreateEmpty() {
    m_Scenes.clear();
    m_Scenes.push_back(Editor::Scene::GetEmpty(m_Context));
    m_ActiveSceneID = 0;

    m_IsReady = true;
}

const std::string Editor::GameProject::GetTitle() const {
    return m_Title;
}

const Editor::Scene* Editor::GameProject::GetActiveScene() const {
    auto it = std::find_if(m_Scenes.begin(), m_Scenes.end(), [&](const Scene& scene) {
        return scene.GetID() == m_ActiveSceneID;
    });

    if (it != m_Scenes.end()) {
        return it._Ptr;
    }
    return nullptr;
}

const std::vector<Editor::Scene> Editor::GameProject::GetScenes() const {
    return m_Scenes;
}
