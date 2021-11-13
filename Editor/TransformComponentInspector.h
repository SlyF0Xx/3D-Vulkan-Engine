#pragma once
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include <TransformComponent.h>
#include "ColoredDragFloat.h"

#include "BaseComponentInspector.h"

namespace Editor {

	class TransformComponentInspector : public BaseComponentInspector {
	public:
		TransformComponentInspector() = delete;
		explicit TransformComponentInspector(const diffusion::Ref<Game>& ctx);

		void OnEvent(const SceneInteractEvent& e) override;
		void RenderContent() override;

	private:
		inline const char* GetTitle() const override;

		bool IsRenderable() const override;

		void ApplyTransform();

	private:
		static inline constexpr const bool	MULTI_ENTITIES_SUPPORT = false;

		diffusion::TransformComponent* m_TransformComponent;

		bool m_AlreadyAppliedChanges	= false;

		bool m_LocationFocused			= false;
		bool m_RotationFocused			= false;
		bool m_ScaleFocused				= false;

		float m_Location[3]				= { 0.f, 0.f, 0.f };
		float m_Rotation[3]				= { 0.f, 0.f, 0.f };
		float m_Scale[3]				= { 0.f, 0.f, 0.f };
	};

}