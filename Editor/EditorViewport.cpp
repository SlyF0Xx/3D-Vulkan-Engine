#include "EditorViewport.h"

#include "BaseComponents/CameraComponent.h"
#include "BaseComponents/Relation.h"
#include "BaseComponents/MeshComponent.h"
#include "BaseComponents/TransformComponent.h"
#include "BaseComponents/DebugComponent.h"
#include "Entities/DebugCube.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include <set>

Editor::EditorViewport::EditorViewport(EDITOR_GAME_TYPE game) : GameWidget(game) {
	m_SnapDispatcher = ViewportSnapInteractionSingleTon::GetDispatcher();
	m_SceneDispatcher = SceneInteractionSingleTon::GetDispatcher();

	m_SceneDispatcher->appendListener(SceneInteractType::SELECTED_ONE, [&](const SceneInteractEvent& e) {
		m_Selection = e.Entities[0];
	});

	m_SceneDispatcher->appendListener(SceneInteractType::RESET_SELECTION, [&](const SceneInteractEvent& e) {
		m_Selection = -1;
	});
}

void Editor::EditorViewport::Render() {
	throw;
}

void Editor::EditorViewport::Render(bool* p_open, ImGuiWindowFlags flags) {
	throw;
}

void Editor::EditorViewport::ClickHandler() {
	ImVec2 origin = ImGui::GetMousePos();
	ImVec2 boundary = ImGui::GetWindowPos();
	m_ScreenSpaceClickCoordsRaw = origin - boundary;

	if (!(m_ScreenSpaceClickCoordsRaw.x < m_SceneSize.x
		&& m_ScreenSpaceClickCoordsRaw.x > 0
		&& m_ScreenSpaceClickCoordsRaw.y < m_SceneSize.y
		&& m_ScreenSpaceClickCoordsRaw.y > 0)) {
		return;
	}

	m_DepthClick = m_Context->get_depth(m_ScreenSpaceClickCoordsRaw.x, m_ScreenSpaceClickCoordsRaw.y);

	m_ScreenSpaceClickCoords = m_ScreenSpaceClickCoordsRaw / m_SceneSize;
	m_ScreenSpaceClickCoordsNorm = (m_ScreenSpaceClickCoords - ImVec2(0.5f, 0.5f)) * 2;

	m_GlobalPosition = diffusion::get_world_point_by_screen(
		m_Context->get_registry(), m_ScreenSpaceClickCoordsNorm.x, m_ScreenSpaceClickCoordsNorm.y, m_DepthClick);

	m_Context->get_registry().view<diffusion::TransformComponent, diffusion::SubMesh>(entt::exclude<diffusion::debug_tag>).each(
		[this](const diffusion::TransformComponent& transform, const diffusion::SubMesh& mesh) {

		if (diffusion::is_in_bounding_box(diffusion::calculate_bounding_box_in_world_space(
			m_Context->get_registry(), mesh, transform), m_GlobalPosition)) {
			auto parrent = entt::to_entity(m_Context->get_registry(), mesh);

			glm::vec3 delta = mesh.m_bounding_box.max - mesh.m_bounding_box.min;
			glm::vec3 delta_2 = glm::vec3(delta.x / 2, delta.y / 2, delta.z / 2);

			auto entity = diffusion::create_debug_cube_entity(
				m_Context->get_registry(),
				mesh.m_bounding_box.min + delta_2,
				glm::vec3(0),
				delta
			);
			m_Context->get_registry().emplace<diffusion::Relation>(entity, parrent);

			m_SceneDispatcher->dispatch(SceneInteractEvent(SceneInteractType::SELECTED_ONE, static_cast<uint32_t>(parrent)));
		}
	});
}

void Editor::EditorViewport::Render(bool* p_open, ImGuiWindowFlags flags, ImGUIBasedPresentationEngine& engine) {
	ImGuiWindowFlags _flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
	_flags |= flags;
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0, 0});
	ImGui::Begin(TITLE, p_open, _flags);

#pragma region Render Target.
	ImVec2 currentSize = ImGui::GetContentRegionAvail();
	static bool resizeNeeded = false;
	if (currentSize.x != m_SceneSize.x || currentSize.y != m_SceneSize.y) {
		m_SceneSize = currentSize;
		resizeNeeded = true;
		// OnResize(m_Context, engine);
	}

	//m_RenderSize = ImGui::GetIO().DisplaySize;

	if (currentSize.x > m_RenderSize.x) {
		float multiplier = (int) (currentSize.x / STEP) + 1;
		m_RenderSize.x = STEP * multiplier;
		resizeNeeded = true;
		//OnResize(m_Context, engine);
	} else if (currentSize.y > m_RenderSize.y) {
		float multiplier = (int) (currentSize.y / STEP) + 1;
		m_RenderSize.y = STEP * multiplier;
		resizeNeeded = true;
		//OnResize(m_Context, engine);
	}
	if (resizeNeeded) {
		OnResizeInternal(m_Context, engine);
	}

	ImVec2 pos = (ImGui::GetWindowSize() - m_RenderSize) * 0.5f;
	ImGui::SetCursorPos(pos);

	if (!m_IsResizingWindow) {
		ImGui::Image(m_TexIDs[m_Context->get_presentation_engine().SemaphoreIndex], m_RenderSize);
	}
#pragma endregion

	m_TopLeftPoint = ImGui::GetWindowContentRegionMin() + ImGui::GetWindowPos();
	auto click = ImGui::IsMouseClicked(0);

#pragma region Viewport Overlay.
	ImGui::SetNextWindowPos(m_TopLeftPoint);
	ImGui::SetNextWindowSize(m_SceneSize);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, Constants::EDITOR_WINDOW_PADDING);
	ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.f);
	ImGui::BeginChild("##Overlay", m_SceneSize, false,
		ImGuiWindowFlags_NoBackground
		| ImGuiWindowFlags_NoTitleBar
		| ImGuiWindowFlags_NoResize
		| ImGuiWindowFlags_NoMove
		| ImGuiWindowFlags_NoScrollbar
		| ImGuiWindowFlags_NoScrollWithMouse
		| ImGuiWindowFlags_AlwaysUseWindowPadding
	);

	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, Constants::OVERLAY_HOVER_COLOR);
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, Constants::OVERLAY_ACTIVE_COLOR);

	int frame_padding = -1;									// -1 == uses default padding (style.FramePadding)
	ImVec2 size = ImVec2(32, 32);     // Size of the image we want to make visible
	ImVec2 uv0 = ImVec2(0.0f, 0.0f);                        // UV coordinates for lower-left
	ImVec2 uv1 = ImVec2(1, 1);								// UV coordinates for (thumbnailSize, thumbnailSize) in our texture
	ImVec4 bg_col = ImVec4(0.f, 0.f, 0.f, .0f);         // Background.
	ImVec4 tint_col = ImVec4(1.0f, 1.f, 1.0f, 1.f);       // No tint
	ImGui::SameLine(m_SceneSize.x - (48.f * 3.f));

	if (m_CurrentGizmoOperation == ImGuizmo::TRANSLATE) {
		ImGui::PushStyleColor(ImGuiCol_Button, Constants::OVERLAY_SUCCESS_COLOR);
	} else {
		ImGui::PushStyleColor(ImGuiCol_Button, Constants::OVERLAY_DEFAULT_COLOR);
	}
	if (ImGui::ImageButton(m_GridTex, size, uv0, uv1, frame_padding, bg_col, tint_col))
		m_CurrentGizmoOperation = ImGuizmo::TRANSLATE;
	if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
		ImGui::OpenPopup(POPUP_TRANSFORM);
	ImGui::PopStyleColor();

	ImGui::SameLine();

	if (m_CurrentGizmoOperation == ImGuizmo::ROTATE) {
		ImGui::PushStyleColor(ImGuiCol_Button, Constants::OVERLAY_SUCCESS_COLOR);
	} else {
		ImGui::PushStyleColor(ImGuiCol_Button, Constants::OVERLAY_DEFAULT_COLOR);
	}
	if (ImGui::ImageButton(m_RotationTex, size, uv0, uv1, frame_padding, bg_col, tint_col))
		m_CurrentGizmoOperation = ImGuizmo::ROTATE;
	if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
		ImGui::OpenPopup(POPUP_ROTATION);
	ImGui::PopStyleColor();

	ImGui::SameLine();

	if (m_CurrentGizmoOperation == ImGuizmo::SCALE) {
		ImGui::PushStyleColor(ImGuiCol_Button, Constants::OVERLAY_SUCCESS_COLOR);
	} else {
		ImGui::PushStyleColor(ImGuiCol_Button, Constants::OVERLAY_DEFAULT_COLOR);
	}
	if (ImGui::ImageButton(m_ScaleTex, size, uv0, uv1, frame_padding, bg_col, tint_col))
		m_CurrentGizmoOperation = ImGuizmo::SCALE;
	if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
		ImGui::OpenPopup(POPUP_SCALE);
	ImGui::PopStyleColor();

	if (ImGui::IsKeyPressed(90))
		m_CurrentGizmoOperation = ImGuizmo::TRANSLATE;
	if (ImGui::IsKeyPressed(69))
		m_CurrentGizmoOperation = ImGuizmo::ROTATE;
	if (ImGui::IsKeyPressed(82)) // r Key
		m_CurrentGizmoOperation = ImGuizmo::SCALE;

	if (ImGui::BeginPopup(POPUP_TRANSFORM)) {
		TransformSnapSize localSize = m_TransformSnapSize;
		bool localCheckbox = m_IsTransformSnap;

		ImGui::PushFont(FontUtils::GetFont(FONT_TYPE::SUBHEADER_TEXT));
		ImGui::Text("Location");
		ImGui::Separator();

		ImGui::Text("Gizmo mode");
		ImGui::PopFont();

		if (ImGui::RadioButton("WORLD", m_CurrentGizmoMode == ImGuizmo::WORLD)) {
			m_CurrentGizmoMode = ImGuizmo::WORLD;
		}
		ImGui::SameLine();
		if (ImGui::RadioButton("LOCAL", m_CurrentGizmoMode == ImGuizmo::LOCAL)) {
			m_CurrentGizmoMode = ImGuizmo::LOCAL;
		}
		ImGui::Separator();

		ImGui::Checkbox("Snap enabled", &m_IsTransformSnap);
		ImGui::Separator();
		if (!m_IsTransformSnap) {
			ImGui::BeginDisabled();
		}
		if (ImGui::RadioButton("1", m_TransformSnapSize == TransformSnapSize::ONE)) {
			m_TransformSnapSize = TransformSnapSize::ONE;
		}
		if (ImGui::RadioButton("5", m_TransformSnapSize == TransformSnapSize::FIVE)) {
			m_TransformSnapSize = TransformSnapSize::FIVE;
		}
		if (ImGui::RadioButton("10", m_TransformSnapSize == TransformSnapSize::TEN)) {
			m_TransformSnapSize = TransformSnapSize::TEN;
		}
		if (ImGui::RadioButton("50", m_TransformSnapSize == TransformSnapSize::FIFTY)) {
			m_TransformSnapSize = TransformSnapSize::FIFTY;
		}
		if (ImGui::RadioButton("100", m_TransformSnapSize == TransformSnapSize::ONE_HUNDRED)) {
			m_TransformSnapSize = TransformSnapSize::ONE_HUNDRED;
		}
		if (ImGui::RadioButton("500", m_TransformSnapSize == TransformSnapSize::FIVE_HUNDRED)) {
			m_TransformSnapSize = TransformSnapSize::FIVE_HUNDRED;
		}
		if (ImGui::RadioButton("1 000", m_TransformSnapSize == TransformSnapSize::ONE_THOUSAND)) {
			m_TransformSnapSize = TransformSnapSize::ONE_THOUSAND;
		}
		if (ImGui::RadioButton("5 000", m_TransformSnapSize == TransformSnapSize::FIVE_THOUSAND)) {
			m_TransformSnapSize = TransformSnapSize::FIVE_THOUSAND;
		}
		if (ImGui::RadioButton("10 000", m_TransformSnapSize == TransformSnapSize::TEN_THOUSAND)) {
			m_TransformSnapSize = TransformSnapSize::TEN_THOUSAND;
		}
		if (!m_IsTransformSnap) {
			ImGui::EndDisabled();
		}

		if (localSize != m_TransformSnapSize || localCheckbox != m_IsTransformSnap) {
			m_SnapDispatcher->dispatch(
				ViewportInteractType::TRANSFORM_SNAP,
				m_IsTransformSnap,
				static_cast<int>(m_TransformSnapSize)
			);
		}
		ImGui::EndPopup();
	} // POPUP_TRANSFORM

	if (ImGui::BeginPopup(POPUP_ROTATION)) {
		RotationSnapSize localSize = m_RotationSnapSize;
		bool localCheckbox = m_IsRotationSnap;

		ImGui::PushFont(FontUtils::GetFont(FONT_TYPE::SUBHEADER_TEXT));
		ImGui::Text("Rotation");
		ImGui::Separator();

		ImGui::Text("Gizmo mode");
		ImGui::PopFont();

		if (ImGui::RadioButton("WORLD", m_CurrentGizmoMode == ImGuizmo::WORLD)) {
			m_CurrentGizmoMode = ImGuizmo::WORLD;
		}
		ImGui::SameLine();
		if (ImGui::RadioButton("LOCAL", m_CurrentGizmoMode == ImGuizmo::LOCAL)) {
			m_CurrentGizmoMode = ImGuizmo::LOCAL;
		}
		ImGui::Separator();

		ImGui::Checkbox("Snap enabled", &m_IsRotationSnap);
		ImGui::Separator();
		if (!m_IsRotationSnap) {
			ImGui::BeginDisabled();
		}
		if (ImGui::RadioButton("5", m_RotationSnapSize == RotationSnapSize::FIVE)) {
			m_RotationSnapSize = RotationSnapSize::FIVE;
		}
		if (ImGui::RadioButton("10", m_RotationSnapSize == RotationSnapSize::TEN)) {
			m_RotationSnapSize = RotationSnapSize::TEN;
		}
		if (ImGui::RadioButton("15", m_RotationSnapSize == RotationSnapSize::FIVETEEN)) {
			m_RotationSnapSize = RotationSnapSize::FIVETEEN;
		}
		if (ImGui::RadioButton("30", m_RotationSnapSize == RotationSnapSize::THIRTY)) {
			m_RotationSnapSize = RotationSnapSize::THIRTY;
		}
		if (ImGui::RadioButton("45", m_RotationSnapSize == RotationSnapSize::FORTY_FIVE)) {
			m_RotationSnapSize = RotationSnapSize::FORTY_FIVE;
		}
		if (ImGui::RadioButton("60", m_RotationSnapSize == RotationSnapSize::SIXTY)) {
			m_RotationSnapSize = RotationSnapSize::SIXTY;
		}
		if (ImGui::RadioButton("90", m_RotationSnapSize == RotationSnapSize::NINETY)) {
			m_RotationSnapSize = RotationSnapSize::NINETY;
		}
		if (ImGui::RadioButton("120", m_RotationSnapSize == RotationSnapSize::ONE_HUNDRED_TWENTY)) {
			m_RotationSnapSize = RotationSnapSize::ONE_HUNDRED_TWENTY;
		}
		if (!m_IsRotationSnap) {
			ImGui::EndDisabled();
		}

		if (localSize != m_RotationSnapSize || localCheckbox != m_IsRotationSnap) {
			m_SnapDispatcher->dispatch(
				ViewportInteractType::ROTATION_SNAP,
				m_IsRotationSnap,
				static_cast<int>(m_RotationSnapSize)
			);
		}
		ImGui::EndPopup();
	} // POPUP_ROTATION

	if (ImGui::BeginPopup(POPUP_SCALE)) {
		ScaleSnapSize localSize = m_ScaleSnapSize;
		bool localCheckbox = m_IsScaleSnap;

		ImGui::PushFont(FontUtils::GetFont(FONT_TYPE::SUBHEADER_TEXT));
		ImGui::Text("Scale");
		ImGui::PopFont();
		ImGui::Separator();

		ImGui::Checkbox("Snap enabled", &m_IsScaleSnap);
		ImGui::Separator();
		if (!m_IsScaleSnap) {
			ImGui::BeginDisabled();
		}
		if (ImGui::RadioButton("1/16", m_ScaleSnapSize == ScaleSnapSize::SIXTEENTH_PART)) {
			m_ScaleSnapSize = ScaleSnapSize::SIXTEENTH_PART;
		}
		if (ImGui::RadioButton("1/8", m_ScaleSnapSize == ScaleSnapSize::EIGTH_PART)) {
			m_ScaleSnapSize = ScaleSnapSize::EIGTH_PART;
		}
		if (ImGui::RadioButton("1/4", m_ScaleSnapSize == ScaleSnapSize::QUATER)) {
			m_ScaleSnapSize = ScaleSnapSize::QUATER;
		}
		if (ImGui::RadioButton("1/2", m_ScaleSnapSize == ScaleSnapSize::HALF)) {
			m_ScaleSnapSize = ScaleSnapSize::HALF;
		}
		if (ImGui::RadioButton("1", m_ScaleSnapSize == ScaleSnapSize::ONE)) {
			m_ScaleSnapSize = ScaleSnapSize::ONE;
		}
		if (ImGui::RadioButton("5", m_ScaleSnapSize == ScaleSnapSize::FIVE)) {
			m_ScaleSnapSize = ScaleSnapSize::FIVE;
		}
		if (ImGui::RadioButton("10", m_ScaleSnapSize == ScaleSnapSize::TEN)) {
			m_ScaleSnapSize = ScaleSnapSize::TEN;
		}
		if (!m_IsScaleSnap) {
			ImGui::EndDisabled();
		}

		if (localSize != m_ScaleSnapSize || localCheckbox != m_IsScaleSnap) {
			m_SnapDispatcher->dispatch(
				ViewportInteractType::SCALE_SNAP,
				m_IsScaleSnap,
				static_cast<int>(m_ScaleSnapSize)
			);
		}
		ImGui::EndPopup();
	}
#ifdef _DEBUG
	ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(127, 127, 250, 255));
	std::string renderSizeStr = "Render size: [X: " + std::to_string(m_RenderSize.x) + " Y: " + std::to_string(m_RenderSize.y) + "]";
	ImGui::Text(renderSizeStr.c_str());

	std::string sceneSizeStr = "Scene size: [X: " + std::to_string(m_SceneSize.x) + " Y: " + std::to_string(m_SceneSize.y) + "]";
	ImGui::Text(sceneSizeStr.c_str());

	if (click) {
		ImGui::Text("Click status: TRUE");
	} else {
		ImGui::Text("Click status: -");
	}

	ImGuiIO& io = ImGui::GetIO();
	std::string screenSpaceMouseCoordsStr = "Screen space mouse coords: [X: " + std::to_string(io.MousePos.x) + " Y: " + std::to_string(io.MousePos.y) + "]";
	ImGui::Text(screenSpaceMouseCoordsStr.c_str());

	std::string screenSpaceRawStr = "Screen space raw click: [X: " + std::to_string(m_ScreenSpaceClickCoordsRaw.x) + " Y: " + std::to_string(m_ScreenSpaceClickCoordsRaw.y) + "]";
	ImGui::Text(screenSpaceRawStr.c_str());

	std::string screenSpaceStr = "Screen space click: [X: " + std::to_string(m_ScreenSpaceClickCoords.x) + " Y: " + std::to_string(m_ScreenSpaceClickCoords.y) + "]";
	ImGui::Text(screenSpaceStr.c_str());

	std::string screenSpaceNormStr = "Screen space norm click: [X: " + std::to_string(m_ScreenSpaceClickCoordsNorm.x) + " Y: " + std::to_string(m_ScreenSpaceClickCoordsNorm.y) + "]";
	ImGui::Text(screenSpaceNormStr.c_str());

	std::string depthStr = "Depth: " + std::to_string(m_DepthClick);
	ImGui::Text(depthStr.c_str());

	std::string globalCoordsStr =
		"Global coords: [X: " + std::to_string(m_GlobalPosition.x) +
		" Y: " + std::to_string(m_GlobalPosition.y) +
		" Z: " + std::to_string(m_GlobalPosition.z) +
		"]";
	ImGui::Text(globalCoordsStr.c_str());

	if (ImGuizmo::IsUsing()) {
		ImGui::Text("Using gizmo");
	} else {
		ImGui::Text(ImGuizmo::IsOver() ? "Over gizmo" : "");
		ImGui::Text(ImGuizmo::IsOver(ImGuizmo::TRANSLATE) ? "Over translate gizmo" : "");
		ImGui::Text(ImGuizmo::IsOver(ImGuizmo::ROTATE) ? "Over rotate gizmo" : "");
		ImGui::Text(ImGuizmo::IsOver(ImGuizmo::SCALE) ? "Over scale gizmo" : "");
	}

	ImGui::Text(("Camera YAW: " + std::to_string(m_CameraYaw)).c_str());
	ImGui::Text(("Camera PITCH: " + std::to_string(m_CameraPitch)).c_str());
	auto& mainCameraEntity = m_Context->get_registry().ctx<diffusion::MainCameraTag>();
	auto& cameraComponent = m_Context->get_registry().get<diffusion::CameraComponent>(mainCameraEntity.m_entity);
	auto& transform = m_Context->get_registry().get<diffusion::TransformComponent>(mainCameraEntity.m_entity);
	auto camera_view = calculate_camera_view(m_Context->get_registry(), cameraComponent, transform);

	std::string targetPosCoordsStr =
		"Target position: [X: " + std::to_string(camera_view.target.x) +
		" Y: " + std::to_string(camera_view.target.y) +
		" Z: " + std::to_string(camera_view.target.z) +
		"]";
	ImGui::Text(targetPosCoordsStr.c_str());

	ImGui::PopStyleColor();
#endif // _DEBUG.

	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
	ImGui::EndChild();
	ImGui::PopStyleVar();
	ImGui::PopStyleVar();
#pragma endregion // Overlay.

	DrawGizmo();
	if (ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
		auto& mainCameraEntity = m_Context->get_registry().ctx<diffusion::MainCameraTag>();
		auto& cameraComponent = m_Context->get_registry().get<diffusion::CameraComponent>(mainCameraEntity.m_entity);

		m_TargetPosition = camera_view.target;
	} else if (!ImGuizmo::IsUsing() && click) {
		ClickHandler();
	}

	if (ImGui::IsMouseDown(ImGuiMouseButton_Right)) {
		RightClickHandler();
	} else {
		m_CameraYaw = 0.0f;
		m_CameraPitch = 0.0f;
	}

	ImGui::End();
	ImGui::PopStyleVar();
}

void Editor::EditorViewport::OnResize(EDITOR_GAME_TYPE vulkan, ImGUIBasedPresentationEngine& engine) {
	m_IsResizingWindow = true;

	float multiplierX = (int) (m_SceneSize.x / STEP) + 1;
	float multiplierY = (int) (m_SceneSize.y / STEP) + 1;
	m_RenderSize = {STEP * multiplierX, STEP * multiplierY};

	engine.resize(m_RenderSize.x, m_RenderSize.y);
	vulkan->InitializePresentationEngine(engine.get_presentation_engine());

	m_TexIDs.clear();
	for (auto& swapchain_data : engine.get_presentation_engine().m_swapchain_data) {
		vk::Sampler color_sampler = vulkan->get_device().createSampler(vk::SamplerCreateInfo({}, vk::Filter::eLinear, vk::Filter::eLinear, vk::SamplerMipmapMode::eLinear, vk::SamplerAddressMode::eClampToEdge, vk::SamplerAddressMode::eClampToEdge, vk::SamplerAddressMode::eClampToEdge, 0, VK_FALSE, 0, VK_FALSE, vk::CompareOp::eAlways, 0, 0, vk::BorderColor::eFloatOpaqueWhite, VK_FALSE));
		m_TexIDs.push_back(ImGui_ImplVulkan_AddTexture(color_sampler, swapchain_data.m_color_image_view, static_cast<VkImageLayout>(vk::ImageLayout::eGeneral)));
	}

	// TODO: Вот тут крашится, на этом мои полномочия всё.
	// vulkan->SecondInitialize();

	m_IsResizingWindow = false;
}

void Editor::EditorViewport::OnResizeInternal(EDITOR_GAME_TYPE vulkan, ImGUIBasedPresentationEngine& engine) {
	engine.resize(m_RenderSize.x, m_RenderSize.y);
	vulkan->InitializePresentationEngine(engine.get_presentation_engine());
	vulkan->SecondInitialize();

	m_TexIDs.clear();
	for (auto& swapchain_data : engine.get_presentation_engine().m_swapchain_data) {
		vk::Sampler color_sampler = vulkan->get_device().createSampler(vk::SamplerCreateInfo({}, vk::Filter::eLinear, vk::Filter::eLinear, vk::SamplerMipmapMode::eLinear, vk::SamplerAddressMode::eClampToEdge, vk::SamplerAddressMode::eClampToEdge, vk::SamplerAddressMode::eClampToEdge, 0, VK_FALSE, 0, VK_FALSE, vk::CompareOp::eAlways, 0, 0, vk::BorderColor::eFloatOpaqueWhite, VK_FALSE));
		m_TexIDs.push_back(ImGui_ImplVulkan_AddTexture(color_sampler, swapchain_data.m_color_image_view, static_cast<VkImageLayout>(vk::ImageLayout::eGeneral)));
	}
}

void Editor::EditorViewport::InitContexed() {
	GameWidget::InitContexed();

	m_Selection = -1;
	m_TexIDs.clear();

	m_GridTex = GenerateTextureID(m_Context, m_GridTexData, GRID_ICON_PATH);
	m_RotationTex = GenerateTextureID(m_Context, m_RotationTexData, ROTATION_ICON_PATH);
	m_ScaleTex = GenerateTextureID(m_Context, m_ScaleTexData, SCALE_ICON_PATH);
}

void Editor::EditorViewport::DrawGizmo(ImDrawList* drawlist) {
	if (!m_Selection || m_Selection == -1)
		return;

	auto& mainCameraEntity = m_Context->get_registry().ctx<diffusion::MainCameraTag>();
	auto& cameraComponent = m_Context->get_registry().get<diffusion::CameraComponent>(mainCameraEntity.m_entity);
	auto& camera_transform = m_Context->get_registry().get<diffusion::TransformComponent>(mainCameraEntity.m_entity);
	auto camera_view = calculate_camera_view(m_Context->get_registry(), cameraComponent, camera_transform);

	auto transformComponent = m_Context->get_registry().try_get<diffusion::TransformComponent>((entt::entity) m_Selection);


	if (transformComponent) {
		ImGuizmo::SetOrthographic(false);
		ImGuizmo::SetDrawlist(drawlist);

		bool isSnapUsing = false;
		static float snap[3] = {1.f, 1.f, 1.f};
		switch (m_CurrentGizmoOperation) {
			case ImGuizmo::TRANSLATE:
			case ImGuizmo::TRANSLATE_X:
			case ImGuizmo::TRANSLATE_Y:
			case ImGuizmo::TRANSLATE_Z:
				isSnapUsing = m_IsTransformSnap;
				snap[0] = snap[1] = snap[2] = GetTransformSpeedBySnapSize(isSnapUsing, m_TransformSnapSize);
				break;
			case ImGuizmo::ROTATE:
			case ImGuizmo::ROTATE_X:
			case ImGuizmo::ROTATE_Y:
			case ImGuizmo::ROTATE_Z:
				isSnapUsing = m_IsRotationSnap;
				snap[0] = GetRotationSpeedBySnapSize(isSnapUsing, m_RotationSnapSize);
				break;
			case ImGuizmo::SCALE:
			case ImGuizmo::SCALE_X:
			case ImGuizmo::SCALE_Y:
			case ImGuizmo::SCALE_Z:
				isSnapUsing = m_IsScaleSnap;
				snap[0] = GetScaleSpeedBySnapSize(isSnapUsing, m_ScaleSnapSize);
				break;
		}

		auto pos = m_TopLeftPoint;
		ImGuizmo::SetRect(pos.x, pos.y, m_SceneSize.x, m_SceneSize.y);

		auto view = glm::lookAtLH(
			camera_view.position,
			camera_view.target,
			-camera_view.up
		);

		auto perspective = glm::perspectiveLH(
			cameraComponent.fov_y,
			cameraComponent.aspect,
			cameraComponent.min_distance,
			cameraComponent.max_distance
		);

		//auto view = cameraComponent.m_camera_matrix;
		//auto perspective = cameraComponent.m_projection_matrix;

		glm::mat4 global_matrix = diffusion::calculate_global_world_matrix(m_Context->get_registry(), *transformComponent);
		glm::mat4 original = global_matrix * glm::inverse(transformComponent->m_world_matrix);

#if _DEBUG && 0
		ImGuizmo::DrawGrid(
			glm::value_ptr(view),
			glm::value_ptr(perspective),
			glm::value_ptr(glm::identity<glm::mat4>()),
			200.f
		);

		ImGuizmo::DrawCubes(glm::value_ptr(view),
			glm::value_ptr(perspective), glm::value_ptr(global_matrix), 1);

		// CAMERA.
		/*ImGuizmo::DrawCubes(
			glm::value_ptr(view),
			glm::value_ptr(perspective),
			glm::value_ptr(
				diffusion::create_matrix(
					cameraComponent.m_camera_position,
					glm::vec3(0, 0, 0),
					glm::vec3(0.5f, 0.5f, 0.5f)
				)
			),
			1);*/
#endif

		ImGuizmo::Manipulate(
			glm::value_ptr(view),
			glm::value_ptr(perspective),
			m_CurrentGizmoOperation,
			m_CurrentGizmoMode,
			glm::value_ptr(global_matrix),
			NULL,
			isSnapUsing ? snap : NULL);

		ImGuizmo::ViewManipulate(glm::value_ptr(view), 10.f, {m_SceneSize.x + m_TopLeftPoint.x - 128, m_SceneSize.y + m_TopLeftPoint.y - 128}, {128.f, 128.f}, Constants::OVERLAY_DEFAULT_COLOR);


		if (ImGuizmo::IsUsing()) {
			m_Context->get_registry().patch<diffusion::TransformComponent>((entt::entity) m_Selection, [global_matrix, &original](diffusion::TransformComponent& transform) {
				glm::mat4 new_end = glm::inverse(original) * global_matrix;

				transform.m_world_matrix = new_end;
			});
		}
	}
}

void Editor::EditorViewport::RightClickHandler() {
	if (!ImGui::IsMousePosValid()) {
		return;
	}
	float scaleMultiplier = -2.5f;
	float moveMultiplier = 1.f;
	float sensitivity = 5.f;


	ImGuiIO& io = ImGui::GetIO();

	auto& mainCameraEntity = m_Context->get_registry().ctx<diffusion::MainCameraTag>();
	auto& cameraComp = m_Context->get_registry().get<diffusion::CameraComponent>(mainCameraEntity.m_entity);
	auto& camera_transform = m_Context->get_registry().get<diffusion::TransformComponent>(mainCameraEntity.m_entity);
	auto camera_view = calculate_camera_view(m_Context->get_registry(), cameraComp, camera_transform);

	if (io.MouseWheel != 0.f) {
		m_Context->get_registry().patch<diffusion::CameraComponent>(
			mainCameraEntity.m_entity, [&](diffusion::CameraComponent& camera) {
			camera.fov_y += glm::radians(io.MouseWheel * scaleMultiplier);

			float fov = glm::degrees(camera.fov_y);
			if (fov <= 1.f) {
				camera.fov_y = glm::radians(1.f);
			} else if (fov >= 75.f) {
				camera.fov_y = glm::radians(75.f);
			}

			camera.fov_x = atan(tan(camera.fov_y / 2) * camera.aspect) * 2;

			camera.m_projection_matrix = glm::perspective(
				camera.fov_y,
				camera.aspect,
				camera.min_distance,
				camera.max_distance
			);

			camera.m_view_projection_matrix = camera.m_projection_matrix * camera.m_camera_matrix;
		});
		return;
	}

	glm::vec3 forward = glm::normalize(camera_view.target - camera_view.position);
	glm::vec3 right = glm::cross(forward, camera_view.up);

	float dx = io.MouseDelta.x * sensitivity * io.DeltaTime;
	float dy = io.MouseDelta.y * sensitivity * io.DeltaTime;

	m_CameraPitch += dy;
	m_CameraYaw += dx;

	if (m_CameraPitch > 89.f) {
		m_CameraPitch = 89.f;
	} else if (m_CameraPitch < -89.f) {
		m_CameraPitch = -89.f;
	}

	if (std::abs(m_CameraPitch) > std::abs(m_CameraYaw)) {
		m_CameraYaw = 0.0f;
	} else {
		m_CameraPitch = 0.0f;
	}

	//front = glm::normalize(front);
	m_Context->get_registry().patch<diffusion::TransformComponent>(
		mainCameraEntity.m_entity, [&](diffusion::TransformComponent& transform) {
		glm::vec3 up = camera_view.up;
		up.z = -up.z;

		transform.m_world_matrix = glm::rotate(transform.m_world_matrix, glm::radians(m_CameraPitch) * 0.01f, right);
		// do not work normally, when delta z != 0
		transform.m_world_matrix = glm::rotate(transform.m_world_matrix, glm::radians(m_CameraYaw) * 0.01f, glm::vec3(0, 0, 1));

	});
}
