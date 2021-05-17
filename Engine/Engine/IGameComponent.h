#pragma once

#ifdef ENGINE_EXPORTS
#define ENGINE_API __declspec(dllexport)
#else
#define ENGINE_API __declspec(dllimport)
#endif

#include <vector>

namespace vk {
    class Semaphore;
    class Image;
}

class ENGINE_API IGameComponent
{
public:
    virtual vk::Semaphore Draw(int swapchain_image_index, vk::Semaphore wait_sema) = 0;
    virtual void Initialize(const std::vector<vk::Image>&) = 0;
    virtual void DestroyResources() = 0;
};