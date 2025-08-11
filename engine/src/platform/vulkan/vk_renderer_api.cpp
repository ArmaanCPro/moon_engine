#include "moonpch.h"
#include "vk_renderer_api.h"

#include "core/application.h"

namespace moon
{
    void vk_renderer_api::init(graphics_context* context)
    {
        m_context = static_cast<vk_context*>(context);
        // load pipelines, descriptor set layouts, shaders
        // create gpu buffers/textures
    }

    void vk_renderer_api::begin_rendering()
    {
        auto cmd = m_context->get_active_command_buffer();
        auto image_view = m_context->get_swapchain().get_image_views()[ m_context->get_swapchain_image_index()];

        vk::RenderingAttachmentInfo colorAttachment{
            image_view, vk::ImageLayout::eColorAttachmentOptimal
        };
        colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
        colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
        colorAttachment.clearValue = vk::ClearColorValue{ m_clear_color.r, m_clear_color.g, m_clear_color.b, m_clear_color.a };

        vk::RenderingInfo renderingInfo{};
        renderingInfo.renderArea = vk::Rect2D{{0, 0}, m_context->get_swapchain().get_extent()};
        renderingInfo.layerCount = 1;
        renderingInfo.colorAttachmentCount = 1;
        renderingInfo.pColorAttachments = &colorAttachment;

        vk::Viewport viewport;
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float)m_context->get_swapchain().get_extent().width;
        viewport.height = (float)m_context->get_swapchain().get_extent().height;

        vk::Rect2D scissor;
        scissor.offset = vk::Offset2D{ 0, 0 };
        scissor.extent = m_context->get_swapchain().get_extent();

        cmd.beginRendering(renderingInfo);

        cmd.setViewport(0, 1, &viewport);
        cmd.setScissor(0, 1, &scissor);
    }

    void vk_renderer_api::end_rendering()
    {
        auto cmd = m_context->get_active_command_buffer();
        cmd.endRendering();
    }

    void vk_renderer_api::set_viewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
    {
        // deprecated
    }

    void vk_renderer_api::set_clear_color(const glm::vec4& color)
    {
        m_clear_color = color;
    }

    void vk_renderer_api::clear()
    {
        // deprecated
    }

    void vk_renderer_api::draw_indexed(const ref<vertex_array>& vertex_array, uint32_t index_count)
    {

    }
}
