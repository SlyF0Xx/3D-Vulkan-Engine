#include "GameProject.h"

Editor::GameProject::GameProject() {
	m_WindowTitleDispatcher = WindowTitleInteractionSingleTon::GetDispatcher();
}

void Editor::GameProject::CreateContext() {
	m_Scenes.clear();
	m_Scenes.push_back(Editor::Scene(0));
	m_ActiveSceneID = 0; // REPLACE FOR SETTER.
}

void Editor::GameProject::CreateEmpty() {
	if (m_Scenes.size() > 1) {
		return;
	}

	m_Scenes[0].Load(m_SourcePath);
	m_ActiveSceneID = 0; // REPLACE FOR SETTER.

	m_WindowTitleDispatcher->dispatch(Editor::WindowTitleInteractionType::TITLE_UPDATED);
}

void Editor::GameProject::NewScene() {
	uint32_t id = m_Scenes.size();
	Scene scene = Scene(id);
	m_Scenes.push_back(scene);
	_SetActiveScene(id);

	scene.Load(m_SourcePath);

	m_WindowTitleDispatcher->dispatch(Editor::WindowTitleInteractionType::TITLE_UPDATED);
}

void Editor::GameProject::RenameScene() {
	m_IsRenamingScene = true;
}

void Editor::GameProject::Render() {
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, Constants::EDITOR_WINDOW_PADDING);
	ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.f);

	// Always center this window when appearing.
	ImVec2 center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

	if (m_IsRenamingScene) {
		ImGui::OpenPopup("Scene Title");
		ImGui::BeginPopupModal("Scene Title", NULL, ImGuiWindowFlags_AlwaysAutoResize);
		ImGui::Text("Enter new scene title...");
		ImGui::Separator();

		static char nameBuf[64] = "";
		ImGui::InputText("Scene title", nameBuf, 64);

		if (ImGui::Button("OK", ImVec2(120, 0))) {
			if (strlen(nameBuf) != 0) {
				m_IsRenamingScene = false;
				ImGui::CloseCurrentPopup();
				GetActiveScene()->m_Title = nameBuf;

				m_WindowTitleDispatcher->dispatch(Editor::WindowTitleInteractionType::TITLE_UPDATED);
			}
		}
		ImGui::SetItemDefaultFocus();
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(120, 0))) {
			m_IsRenamingScene = false;
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}

	if (m_IsSavingAs) {
		ImGui::OpenPopup("Save Project");
		ImGui::BeginPopupModal("Save Project", NULL, ImGuiWindowFlags_AlwaysAutoResize);
		ImGui::Text("Project name...");
		ImGui::Separator();

		static char nameBuf[64] = "";
		ImGui::InputText("Project name", nameBuf, 64);

		if (ImGui::Button("Continue", ImVec2(120, 0))) {
			if (strlen(nameBuf) != 0) {
				m_IsSavingAs = false;
				ImGui::CloseCurrentPopup();
				m_Title = nameBuf;

#if _DEBUG
				printf("Chosen project name... ");
				printf(m_Title.c_str());
				printf("\n");
#endif

				Save();
			}
		}
		ImGui::SetItemDefaultFocus();
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(120, 0))) {
			m_IsSavingAs = false;
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}

	if (m_IsSourceFolderChoosing) {
		ImGuiFileDialog::Instance()->OpenModal("ChooseDir", "Choose source folder", nullptr, ".");

		if (ImGuiFileDialog::Instance()->Display("ChooseDir")) {
			if (ImGuiFileDialog::Instance()->IsOk()) {
				std::string dirPath = ImGuiFileDialog::Instance()->GetCurrentPath();

#if _DEBUG
				printf("Saving to dir... ");
				printf(dirPath.c_str());
				printf("\n");
#endif

				m_SourcePath = std::filesystem::path(dirPath);
				Save();
			}

			m_IsSourceFolderChoosing = false;
			ImGuiFileDialog::Instance()->Close();
		}
	}

	if (m_IsProjectFileChoosing) {
		ImGuiFileDialog::Instance()->OpenModal("ChooseFile", "Choose project file", ".diffusion", ".");

		if (ImGuiFileDialog::Instance()->Display("ChooseFile")) {
			if (ImGuiFileDialog::Instance()->IsOk()) {
				std::string dirPath = ImGuiFileDialog::Instance()->GetCurrentPath();
				std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();

#if _DEBUG
				printf("Loading from... ");
				printf(filePathName.c_str());
				printf("\n");
#endif
				m_IsProjectFileChoosing = false;
				ImGuiFileDialog::Instance()->Close();

				m_SourcePath = std::filesystem::path(dirPath);
				ParseMetaFile(std::filesystem::path(filePathName));
			}
			m_IsProjectFileChoosing = false;
			ImGuiFileDialog::Instance()->Close();
		}
	}

	ImGui::PopStyleVar();
	ImGui::PopStyleVar();
}

void Editor::GameProject::Load() {
	m_IsProjectFileChoosing = true;
}

void Editor::GameProject::Save() {
	if (m_SourcePath.empty()) {
		m_IsSourceFolderChoosing = true;
		return;
	}

	nlohmann::json general;
	general["Title"] = m_Title;
	general["ScenesCount"] = m_Scenes.size();
	general["ActiveSceneID"] = m_ActiveSceneID;

	nlohmann::json scenes;
	for (Scene& source : m_Scenes) {
		nlohmann::json scene;

		scene["ID"] = source.GetID();
		scene["Title"] = source.GetTitle();
		scene["FileName"] = source.GetFileName();

		scenes.push_back(scene);

		source.Save(m_SourcePath);
	}
	general["Scenes"] = scenes;

	std::ofstream fout(GetMetaPath());
	fout << general;
	fout.close();

	m_WindowTitleDispatcher->dispatch(Editor::WindowTitleInteractionType::TITLE_UPDATED);
}

void Editor::GameProject::SaveAs() {
	m_IsSavingAs = true;
}

void Editor::GameProject::SetActiveScene(uint32_t id) {
	auto it = std::find_if(m_Scenes.begin(), m_Scenes.end(), [&](const Scene& scene) {
		return scene.GetID() == id;
	});

	if (it == m_Scenes.end()) {
		printf(("Scene #" + std::to_string(id) + " not found!").c_str());
		return;
	}

	_SetActiveScene(id);
}

void Editor::GameProject::Refresh() {
	GetActiveScene()->Load(m_SourcePath);
}

const bool Editor::GameProject::HasSourceRoot() const {
	return !m_SourcePath.empty();
}

const std::string Editor::GameProject::GetTitle() const {
	return m_Title;
}

Editor::Scene* Editor::GameProject::GetActiveScene() const {
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

const std::filesystem::path Editor::GameProject::GetMetaPath() const {
	if (!m_SourcePath.empty()) {
		return m_SourcePath / (m_Title + ".diffusion");
	}
	return std::filesystem::path();
}

const std::filesystem::path Editor::GameProject::GetSourceRoot() const {
	return m_SourcePath;
}

void Editor::GameProject::ParseMetaFile(std::filesystem::path path) {
	m_Scenes.clear();
	std::ifstream fin(path);

	nlohmann::json general;
	fin >> general;

	m_Title = general["Title"];
	nlohmann::json scenes = general["Scenes"];
	for (nlohmann::json& sceneData : scenes) {
		Scene scene = Scene(0);
		scene.SetData(sceneData);
		m_Scenes.push_back(scene);
	}
	
	uint32_t id = 0;
	if (general.contains("ActiveSceneID")) {
		uint32_t id = general["ActiveSceneID"];
	}

	fin.close();
	_SetActiveScene(id);
}

void Editor::GameProject::_SetActiveScene(uint32_t id) {
	m_ActiveSceneID = id;
	m_WindowTitleDispatcher->dispatch(Editor::WindowTitleInteractionType::CONTEXT_CHANGED);
	// m_WindowTitleDispatcher->dispatch(Editor::WindowTitleInteractionType::TITLE_UPDATED);
}

//diffusion::Ref<Game> Editor::GameProject::GetCurrentContext() const {
//	auto scene = GetActiveScene();
//	if (scene) {
//		return scene->GetContext();
//	}
//	throw;
//}

Game* Editor::GameProject::GetCurrentContext() const {
	auto scene = GetActiveScene();
	if (scene) {
		return scene->GetContext();
	}
	throw;
}