#pragma once
#include <eventpp/eventdispatcher.h>
#include <Core/Base.h>

namespace Editor {

	enum class TransformSnapSize {
		ONE, FIVE, TEN, FIFTY, ONE_HUNDRED, FIVE_HUNDRED, ONE_THOUSAND, FIVE_THOUSAND, TEN_THOUSAND
	};

	enum class RotationSnapSize {
		FIVE, TEN, FIVETEEN, THIRTY, FORTY_FIVE, SIXTY, NINETY, ONE_HUNDRED_TWENTY
	};

	enum class ScaleSnapSize {
		TEN, FIVE, ONE, HALF, QUATER, EIGTH_PART, SIXTEENTH_PART
	};

	enum class ViewportInteractType {
		TRANSFORM_SNAP, ROTATION_SNAP, SCALE_SNAP
	};

	struct ViewportInteractEventPolicies {
		static ViewportInteractType getEvent(const ViewportInteractType& type, const bool enabled, const int value) {
			return type;
		}
	};

	typedef void ViewportInteractFunc(const ViewportInteractType&, const bool, const int);
	using ViewportEventDispatcherSrc = eventpp::EventDispatcher<ViewportInteractType, ViewportInteractFunc, ViewportInteractEventPolicies>;
	using ViewportEventDispatcher = diffusion::Ref<ViewportEventDispatcherSrc>;

	class ViewportSnapInteractionSingleTon {
	public:
		static ViewportEventDispatcher GetDispatcher();
	private:
		static ViewportEventDispatcher s_Dispatcher;
	};

	static float GetTransformSpeedBySnapSize(bool enabled, TransformSnapSize s) {
		if (!enabled) {
			return 0.01f;
		}

		switch (s) {
			case TransformSnapSize::ONE:
				return 1.f;
			case TransformSnapSize::FIVE:
				return 5.f;
			case TransformSnapSize::TEN:
				return 10.f;
			case TransformSnapSize::FIFTY:
				return 50.f;
			case TransformSnapSize::ONE_HUNDRED:
				return 100.f;
			case TransformSnapSize::FIVE_HUNDRED:
				return 500.f;
			case TransformSnapSize::ONE_THOUSAND:
				return 1000.f;
			case TransformSnapSize::FIVE_THOUSAND:
				return 5000.f;
			case TransformSnapSize::TEN_THOUSAND:
				return 10000.f;
			default:
				return 1.f;
		}
	}

	static float GetRotationSpeedBySnapSize(bool enabled, RotationSnapSize s) {
		if (!enabled) {
			return 0.01f;
		}

		switch (s) {
			case RotationSnapSize::FIVE:
				return 5.f;
			case RotationSnapSize::TEN:
				return 10.f;
			case RotationSnapSize::FIVETEEN:
				return 15.f;
			case RotationSnapSize::THIRTY:
				return 30.f;
			case RotationSnapSize::FORTY_FIVE:
				return 45.f;
			case RotationSnapSize::SIXTY:
				return 60.f;
			case RotationSnapSize::NINETY:
				return 90.f;
			case RotationSnapSize::ONE_HUNDRED_TWENTY:
				return 120.f;
			default:
				return 1.f;
		}
	}

	static float GetScaleSpeedBySnapSize(bool enabled, ScaleSnapSize s) {
		if (!enabled) {
			return 0.01f;
		}

		switch (s) {
			case ScaleSnapSize::FIVE:
				return 5.f;
			case ScaleSnapSize::TEN:
				return 10.f;
			case ScaleSnapSize::ONE:
				return 1.f;
			case ScaleSnapSize::HALF:
				return 0.5f;
			case ScaleSnapSize::QUATER:
				return 0.25f;
			case ScaleSnapSize::EIGTH_PART:
				return 0.125f;
			case ScaleSnapSize::SIXTEENTH_PART:
				return 0.0625f;
			default:
				return 1.f;
		}
	}
}