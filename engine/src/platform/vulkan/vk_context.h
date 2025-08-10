#pragma once

#include "core/window.h"
#include "renderer/graphics_context.h"

#include "vk.h"

namespace moon
{
    class vk_context final : public graphics_context
    {
    public:
        vk_context(const native_handle& window);

        void init() override;
        void swap_buffers() override;

    private:
        GLFWwindow* m_glfwwindow;

        vk::Instance m_instance;
        vk::PhysicalDevice m_physical_device;
        vk::Device m_device;
    };
}
