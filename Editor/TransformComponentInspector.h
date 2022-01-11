#pragma once
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include "BaseComponents/TransformComponent.h"
#include "ColoredDragFloat.h"

#include "BaseComponentInspector.h"
#include "ViewportSnapInteraction.h"

namespace Editor {

	class TransformComponentInspector : public BaseComponentInspector {
	public:
		TransformComponentInspector() = delete;
		explicit TransformComponentInspector(EDITOR_GAME_TYPE ctx);

		void RenderContent() override;
		void OnRegisterUpdated() override;
	private:
		inline const char* GetTitle() const override;

		bool IsRenderable() const override;

		void ApplyTransform();

		void OnRemoveComponent() override;

	private:
		static inline constexpr const bool	MULTI_ENTITIES_SUPPORT = false;

		diffusion::TransformComponent* m_TransformComponent;

		SceneEventDispatcher m_SceneDispatcher;
		ViewportEventDispatcher m_SnapDispatcher;

		bool m_AlreadyAppliedChanges	= false;

		bool m_LocationFocused			= false;
		bool m_RotationFocused			= false;
		bool m_ScaleFocused				= false;

		float m_Location[3]				= { 0.f, 0.f, 0.f };
		float m_Rotation[3]				= { 0.f, 0.f, 0.f };
		float m_Scale[3]				= { 0.f, 0.f, 0.f };

		bool m_IsTransformSnap = true;
		bool m_IsRotationSnap = true;
		bool m_IsScaleSnap = true;

		TransformSnapSize m_TransformSnapSize = TransformSnapSize::ONE;
		RotationSnapSize m_RotationSnapSize = RotationSnapSize::FIVE;
		ScaleSnapSize m_ScaleSnapSize = ScaleSnapSize::HALF;
	};

}