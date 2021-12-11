#pragma once

#define IMGUI_DEFINE_MATH_OPERATORS

#include <entt/entt.hpp>
#include <glm/common.hpp>
#include <BaseComponents/CameraComponent.h>
#include <BaseComponents/PossessedComponent.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include "GameWidget.h"
#include "Constants.h"
#include "SceneInteraction.h"
#include "ViewportSnapInteraction.h"
#include "FontUtils.h"

#include <ImGuizmo.h>

namespace Editor {

	class EditorViewport : public GameWidget {
	public:
		static inline constexpr const char* TITLE = "Viewport";

		EditorViewport() = delete;
		EditorViewport(const diffusion::Ref<Game>& game);

		void Render() override;
		void Render(bool* p_open, ImGuiWindowFlags flags) override;
		void Render(bool* p_open, ImGuiWindowFlags flags, ImGUIBasedPresentationEngine& engine);
		void OnResize(Game& vulkan, ImGUIBasedPresentationEngine& engine) override;
		void InitContexed() override;

		void ClickHandler();

		ViewportEventDispatcher GetDispatcher() const;
		void DrawGizmo(ImDrawList* drawlist = nullptr);
	private:
		const float STEP = 30.f;
		const ImVec2 ASPECT_RATIO = {16.f, 9.f};
		const ImVec2 MIN_RENDER_SIZE = {STEP * ASPECT_RATIO.x, STEP * ASPECT_RATIO.y}; // 1920x1080.

		static inline constexpr const char* POPUP_TRANSFORM = "POPUP_TRANSFORM";
		static inline constexpr const char* POPUP_ROTATION = "POPUP_ROTATION";
		static inline constexpr const char* POPUP_SCALE = "POPUP_SCALE";

		static inline constexpr const char* GRID_ICON_PATH = "./misc/icons/grid.png";
		static inline constexpr const char* ROTATION_ICON_PATH = "./misc/icons/rotation.png";
		static inline constexpr const char* SCALE_ICON_PATH = "./misc/icons/scale.png";

		diffusion::ImageData m_GridTexData;
		ImTextureID m_GridTex;

		diffusion::ImageData m_RotationTexData;
		ImTextureID m_RotationTex;

		diffusion::ImageData m_ScaleTexData;
		ImTextureID m_ScaleTex;

		ImVec2 m_SceneSize, m_TopLeftPoint;
		ImVec2 m_RenderSize = MIN_RENDER_SIZE;
		ImVec2 m_ScreenSpaceClickCoordsRaw, m_ScreenSpaceClickCoords, m_ScreenSpaceClickCoordsNorm;
		glm::vec3 m_GlobalPosition;
		double m_DepthClick = 0;
		std::vector<ImTextureID> m_TexIDs;

		bool m_IsTransformSnap = true;
		bool m_IsRotationSnap = true;
		bool m_IsScaleSnap = true;
		TransformSnapSize m_TransformSnapSize = TransformSnapSize::ONE;
		RotationSnapSize m_RotationSnapSize = RotationSnapSize::FIVE;
		ScaleSnapSize m_ScaleSnapSize = ScaleSnapSize::HALF;
		ImGuizmo::OPERATION m_CurrentGizmoOperation = ImGuizmo::TRANSLATE;
		ImGuizmo::MODE m_CurrentGizmoMode = ImGuizmo::WORLD;

		/// <summary>
		/// Выбранная сущность.
		/// Сущность может быть выбрана через Viewport или SceneHierarchy.
		/// </summary>
		ENTT_ID_TYPE m_Selection = -1;

		SceneEventDispatcher m_SceneDispatcher;
		ViewportEventDispatcher m_SnapDispatcher;
	};

}
