#include "moonpch.h"

#include "vk_context.h"

#include <GLFW/glfw3.h>

namespace moon
{
    vk_context::vk_context(const native_handle& window)
    {
        init();
    }

    void vk_context::init()
    {
        vk::ApplicationInfo appInfo("Moon Engine", 1, "Moon Engine", 1, VK_API_VERSION_1_3);

        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        vk::InstanceCreateInfo instanceCreateinfo{
            {}, &appInfo, 0, nullptr, glfwExtensionCount, glfwExtensions
        };

        vk::UniqueInstance instance = vk::createInstanceUnique(instanceCreateinfo);

        // use glfw proc addr loader to initialize dispatch
        vk::detail::DispatchLoaderDynamic dldi{
            instance.get(), [](VkInstance inst, const char* name)
            {
                return glfwGetInstanceProcAddress(inst, name);
            }
        };

        auto devices = instance->enumeratePhysicalDevices(dldi);
    }

    void vk_context::swap_buffers()
    {}
}
