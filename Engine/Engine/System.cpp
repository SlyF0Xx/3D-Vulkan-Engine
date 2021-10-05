#include "System.h"

namespace diffusion {

void System::components_callback(const std::vector<std::reference_wrapper<Component>>& components)
{
	for (auto& component : components) {
		component_callback(component);
	}
}

void System::tick()
{
	components_callback(s_component_manager_instance.get_components_by_tags(m_tags));
}

} // namespace diffusion {