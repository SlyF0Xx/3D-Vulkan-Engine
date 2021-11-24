#include "EditorViewport.h"

Editor::EditorViewport::EditorViewport(const diffusion::Ref<Game>& game) : GameWidget(game) {
	m_SnapDispatcher = diffusion::CreateRef<ViewportEventDispatcherSrc>();
}

void Editor::EditorViewport::Render() {
	throw;
}

void Editor::EditorViewport::Render(bool* p_open, ImGuiWindowFlags flags) {
	throw;
}

void Editor::EditorViewport::Render(bool* p_open, ImGuiWindowFlags flags, ImGUIBasedPresentationEngine& engine) {
	ImGuiWindowFlags _flags = ImGuiWindowFlags_NoScrollbar;
	_flags |= flags;
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0, 0});
	ImGui::Begin(TITLE, p_open, _flags);

	ImVec2 currentSize = ImGui::GetContentRegionAvail();
	if (currentSize.x != m_SceneSize.x || currentSize.y != m_SceneSize.y) {
		m_SceneSize = currentSize;

		OnResize(*m_Context, engine);
	}

	if (currentSize.x > m_RenderSize.x) {
		float multiplier = (int) (currentSize.x / STEP) + 1;
		m_RenderSize.x = STEP * multiplier;
		OnResize(*m_Context, engine);
	} else if (currentSize.y > m_RenderSize.y) {
		float multiplier = (int) (currentSize.y / STEP) + 1;
		m_RenderSize.y = STEP * multiplier;
		OnResize(*m_Context, engine);
	}
	/*float y = glm::max(current_size.x / 16 * 9, current_size.y);
	float x = glm::max(current_size.x, current_size.y / 9 * 16);
	ImVec2 renderSize = {1920, 1080};*/
	/*if (current_size.x * 9 > current_size.y * 16) {
		renderSize = {current_size.x, current_size.x / 16 * 9};
	} else {
		renderSize = {current_size.y / 9 * 16, current_size.y};
	}*/


	//ImGui::PushItemWidth(-1);
	ImVec2 pos = (ImGui::GetWindowSize() - m_RenderSize) * 0.5f;
	ImGui::SetCursorPos(pos);

	ImGui::Image(m_TexIDs[m_Context->get_presentation_engine().SemaphoreIndex], m_RenderSize);


	ImGui::SetNextWindowPos(ImGui::GetWindowContentRegionMin() + ImGui::GetWindowPos());
	ImGui::SetNextWindowSize(m_SceneSize);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, Constants::EDITOR_WINDOW_PADDING);
	ImGui::BeginChild("##Overlay", m_SceneSize, false,
		ImGuiWindowFlags_NoBackground
		| ImGuiWindowFlags_NoTitleBar
		| ImGuiWindowFlags_NoResize
		| ImGuiWindowFlags_NoMove
		| ImGuiWindowFlags_NoScrollbar
		| ImGuiWindowFlags_NoScrollWithMouse
		| ImGuiWindowFlags_AlwaysUseWindowPadding
	);
	ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(210, 240, 110, 160));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(180, 250, 32, 190));

	int frame_padding = -1;									// -1 == uses default padding (style.FramePadding)
	ImVec2 size = ImVec2(16, 16);     // Size of the image we want to make visible
	ImVec2 uv0 = ImVec2(0.0f, 0.0f);                        // UV coordinates for lower-left
	ImVec2 uv1 = ImVec2(1, 1);								// UV coordinates for (thumbnailSize, thumbnailSize) in our texture
	ImVec4 bg_col = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);         // Background.
	ImVec4 tint_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);       // No tint
	ImGui::SameLine(m_SceneSize.x - (32.f * 3.f));
	if (ImGui::ImageButton(m_GridTex, size, uv0, uv1, frame_padding, bg_col, tint_col))
		ImGui::OpenPopup(POPUP_TRANSFORM);
	ImGui::SameLine();
	if (ImGui::ImageButton(m_RotationTex, size, uv0, uv1, frame_padding, bg_col, tint_col))
		ImGui::OpenPopup(POPUP_ROTATION);
	ImGui::SameLine();
	if (ImGui::ImageButton(m_ScaleTex, size, uv0, uv1, frame_padding, bg_col, tint_col))
		ImGui::OpenPopup(POPUP_SCALE);

	if (ImGui::BeginPopup(POPUP_TRANSFORM)) {
		TransformSnapSize localSize = m_TransformSnapSize;
		bool localCheckbox = m_IsTransformSnap;

		ImGui::PushFont(FontUtils::GetFont(FONT_TYPE::SUBHEADER_TEXT));
		ImGui::Text("Location");
		ImGui::PopFont();

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
		ImGui::PopFont();

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
	ImGui::PopStyleColor();
#endif

	/*int gcd = std::gcd((int) renderSize.x, (int) renderSize.y);
	ImGui::Text(std::to_string(gcd).c_str());
	std::string renderSizePropStr = "Render proportions: [" + std::to_string(renderSize.x / gcd) + ":" + std::to_string(renderSize.y / gcd) + "]";
	ImGui::Text(renderSizePropStr.c_str());*/

	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
	ImGui::EndChild();
	ImGui::PopStyleVar();


	//ImVec2 p1 = ImGui::GetCursorScreenPos();
	//ImVec2 p2 = ImVec2(p1.x + 200, p1.y + 200);
	////ImGui::Checkbox("Menu", &menu);
	//ImGui::GetWindowDrawList()->AddLine(p1, p2, IM_COL32(255, 0, 255, 255));
	//ImGui::GetWindowDrawList()->AddCircleFilled(p1, 6.0f, IM_COL32(255, 0, 255, 255));
	//ImGui::GetWindowDrawList()->AddCircleFilled(p2, 6.0f, IM_COL32(255, 0, 255, 255));

	//ImGui::ShowDemoWindow();

	//
	ImGui::End();
	ImGui::PopStyleVar();
}

void Editor::EditorViewport::OnResize(Game& vulkan, ImGUIBasedPresentationEngine& engine) {
	//engine.resize(m_SceneSize.x, m_SceneSize.y);
	engine.resize(m_RenderSize.x, m_RenderSize.y);
	vulkan.InitializePresentationEngine(engine.get_presentation_engine());
	vulkan.SecondInitialize();

	m_TexIDs.clear();
	for (auto& swapchain_data : engine.get_presentation_engine().m_swapchain_data) {
		vk::Sampler color_sampler = vulkan.get_device().createSampler(vk::SamplerCreateInfo({}, vk::Filter::eLinear, vk::Filter::eLinear, vk::SamplerMipmapMode::eLinear, vk::SamplerAddressMode::eClampToEdge, vk::SamplerAddressMode::eClampToEdge, vk::SamplerAddressMode::eClampToEdge, 0, VK_FALSE, 0, VK_FALSE, vk::CompareOp::eAlways, 0, 0, vk::BorderColor::eFloatOpaqueWhite, VK_FALSE));
		m_TexIDs.push_back(ImGui_ImplVulkan_AddTexture(color_sampler, swapchain_data.m_color_image_view, static_cast<VkImageLayout>(vk::ImageLayout::eGeneral)));
	}
}

void Editor::EditorViewport::InitContexed() {
	GameWidget::InitContexed();

	m_GridTex = GenerateTextureID(m_Context, m_GridTexData, GRID_ICON_PATH);
	m_RotationTex = GenerateTextureID(m_Context, m_RotationTexData, ROTATION_ICON_PATH);
	m_ScaleTex = GenerateTextureID(m_Context, m_ScaleTexData, SCALE_ICON_PATH);
}

ViewportEventDispatcher Editor::EditorViewport::GetDispatcher() const {
	return m_SnapDispatcher;
}
