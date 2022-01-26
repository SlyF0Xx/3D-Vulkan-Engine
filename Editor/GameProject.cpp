#include "GameProject.h"

Editor::GameProject::GameProject() {
	m_SceneDispatcher = SceneInteractionSingleTon::GetDispatcher();
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

	m_Scenes[m_AutoIncrement].Load(m_SourceDirectoryPath);
	m_ActiveSceneID = m_AutoIncrement; // REPLACE FOR SETTER.
	m_AutoIncrement++;

	m_WindowTitleDispatcher->dispatch(Editor::WindowTitleInteractionType::TITLE_UPDATED);
}

void Editor::GameProject::NewScene() {
	uint32_t id = m_AutoIncrement++;
	Scene scene = Scene(id);
	m_Scenes.push_back(scene);

	// scene.Load(m_SourcePath);

	_SetActiveScene(id);
}

void Editor::GameProject::RenameScene() {
	m_IsPrepareWindowRequired = true;
	m_IsRenamingScene = true;
}

Editor::GameProjectRenderStatus Editor::GameProject::Render() {
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, Constants::EDITOR_WINDOW_PADDING);
	ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.f);

	if (m_IsPrepareWindowRequired) {
		// Always center this window when appearing.
		ImVec2 center = ImGui::GetMainViewport()->GetCenter();
		ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
		ImGui::SetNextWindowSize({400.f, 600.f});

		m_IsPrepareWindowRequired = false;
	}

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

				m_SourceDirectoryPath = std::filesystem::path(dirPath);
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
				m_SourceDirectoryPath = std::filesystem::path(
					ImGuiFileDialog::Instance()->GetCurrentPath());
				m_SourceProjectFilePath = std::filesystem::path(
					ImGuiFileDialog::Instance()->GetFilePathName());

#if _DEBUG
				printf("Loading from... ");
				printf(m_SourceProjectFilePath.string().c_str());
				printf("\n");
#endif

				m_IsProjectFileChoosing = false;
				ImGuiFileDialog::Instance()->Close();

				ImGui::PopStyleVar();
				ImGui::PopStyleVar();

				return Editor::GameProjectRenderStatus::LOAD_PROJECT;
			}

			m_IsProjectFileChoosing = false;
			ImGuiFileDialog::Instance()->Close();
		}
	}

	if (m_IsDeletingScene) {
		ImGui::OpenPopup("Delete Current Scene");
		ImGui::BeginPopupModal("Delete Current Scene", NULL, ImGuiWindowFlags_AlwaysAutoResize);
		ImGui::Text(("You want to delete scene (" + GetActiveScene()->GetTitle() + ")").c_str());

		ImGui::Text("Are you sure? "); ImGui::SameLine();
		ImGui::PushStyleColor(ImGuiCol_Text, Constants::ERROR_COLOR);
		ImGui::Text("It can't be undone!");

		if (ImGui::Button("[X] DELETE", ImVec2(120, 0))) {
			m_IsDeletingScene = false;

			ImGui::PopStyleColor();
			ImGui::CloseCurrentPopup();
			ImGui::EndPopup();

			ImGui::PopStyleVar();
			ImGui::PopStyleVar();

			return Editor::GameProjectRenderStatus::DELETE_SCENE;
		}
		ImGui::PopStyleColor();

		ImGui::SetItemDefaultFocus();
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(120, 0))) {
			m_IsDeletingScene = false;
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}

	ImGui::PopStyleVar();
	ImGui::PopStyleVar();

	return Editor::GameProjectRenderStatus::SUCCESS;
}

void Editor::GameProject::Load() {
	m_IsPrepareWindowRequired = true;
	m_IsProjectFileChoosing = true;
}

void Editor::GameProject::Save() {
	m_SceneDispatcher->dispatch(SceneInteractType::SAVE_ALL_SCTIPTS);

	if (m_SourceDirectoryPath.empty()) {
		m_IsPrepareWindowRequired = true;
		m_IsSourceFolderChoosing = true;
		return;
	}

	nlohmann::json general;
	general["Title"] = m_Title;
	general["AutoIncrement"] = m_AutoIncrement;
	general["ScenesCount"] = m_Scenes.size();
	general["ActiveSceneID"] = m_ActiveSceneID;

	nlohmann::json scenes;
	for (Scene& source : m_Scenes) {
		nlohmann::json scene;

		scene["ID"] = source.GetID();
		scene["Title"] = source.GetTitle();
		scene["FileName"] = source.GetFileName();

		scenes.push_back(scene);

		source.Save(m_SourceDirectoryPath);
	}
	general["Scenes"] = scenes;

	std::ofstream fout(GetMetaPath());
	fout << general;
	fout.close();

	m_WindowTitleDispatcher->dispatch(Editor::WindowTitleInteractionType::TITLE_UPDATED);
}

void Editor::GameProject::SaveAs() {
	m_IsPrepareWindowRequired = true;
	m_IsSavingAs = true;
}

bool Editor::GameProject::PrepareChangeScene(uint32_t id) {
	if (id == m_ActiveSceneID) {
		return false;
	}
	m_NextActiveSceneID = id;
	return true;
}

void Editor::GameProject::ActivatePreparedScene() {
	_SetActiveScene(m_NextActiveSceneID);
	m_NextActiveSceneID = 0;
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
	GetActiveScene()->Load(m_SourceDirectoryPath);
}

void Editor::GameProject::ParseMetaFile() {
	m_Scenes.clear();
	std::ifstream fin(m_SourceProjectFilePath);

	nlohmann::json general;
	fin >> general;

	m_Title = general["Title"];
	m_AutoIncrement = general["AutoIncrement"];
	nlohmann::json scenes = general["Scenes"];
	for (nlohmann::json& sceneData : scenes) {
		Scene scene = Scene(0);
		scene.SetData(sceneData);
		m_Scenes.push_back(scene);
	}

	uint32_t id = 0;
	if (general.contains("ActiveSceneID")) {
		id = general["ActiveSceneID"];
	}

	fin.close();
	_SetActiveScene(id);
}

void Editor::GameProject::DeleteScene() {
	m_IsDeletingScene = true;
}

void Editor::GameProject::DeleteSceneConfirm() {
	auto it = std::find_if(m_Scenes.begin(), m_Scenes.end(), [&](const Scene& scene) {
		return scene.GetID() == m_ActiveSceneID;
	});

	if (it != m_Scenes.end()) {
		m_Scenes.erase(it);
		if (m_Scenes.size() > 0) {
			_SetActiveScene(m_Scenes[0].GetID());
		} else {
			NewScene();
		}
	}
}

const bool Editor::GameProject::HasSourceRoot() const {
	return !m_SourceDirectoryPath.empty();
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
	if (!m_SourceDirectoryPath.empty()) {
		return m_SourceDirectoryPath / (m_Title + ".diffusion");
	}
	return std::filesystem::path();
}

const std::filesystem::path Editor::GameProject::GetSourceRoot() const {
	return m_SourceDirectoryPath;
}

bool Editor::GameProject::IsRunning() const {
	return !GetCurrentContext()->m_stopped;
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