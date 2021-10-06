#include "InputSystem.h"

#include "ComponentManager.h"
#include "CameraComponent.h"

namespace diffusion {

InputSystem::InputSystem()
	: System({})
{
}

void InputSystem::move_forward()
{
	static_cast<CameraComponent&>(s_component_manager_instance.get_components_by_tag(CameraComponent::s_main_camera_component_tag)[0].get()).move_forward(0.1f);
}

void InputSystem::move_backward()
{
	static_cast<CameraComponent&>(s_component_manager_instance.get_components_by_tag(CameraComponent::s_main_camera_component_tag)[0].get()).move_backward(0.1f);
}

void InputSystem::move_left()
{
	static_cast<CameraComponent&>(s_component_manager_instance.get_components_by_tag(CameraComponent::s_main_camera_component_tag)[0].get()).move_left(0.1f);
}

void InputSystem::move_right()
{
	static_cast<CameraComponent&>(s_component_manager_instance.get_components_by_tag(CameraComponent::s_main_camera_component_tag)[0].get()).move_right(0.1f);
}

void InputSystem::move_up()
{
	static_cast<CameraComponent&>(s_component_manager_instance.get_components_by_tag(CameraComponent::s_main_camera_component_tag)[0].get()).move_up(0.1f);
}

void InputSystem::move_down()
{
	static_cast<CameraComponent&>(s_component_manager_instance.get_components_by_tag(CameraComponent::s_main_camera_component_tag)[0].get()).move_down(0.1f);
}

} // namespace diffusion {