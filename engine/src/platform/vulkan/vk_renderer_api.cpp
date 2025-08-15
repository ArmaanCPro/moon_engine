#include "moonpch.h"
#include "vk_renderer_api.h"

#include "core/application.h"

namespace moon::vulkan
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

        vk::RenderingAttachmentInfo colorAttachment{
            m_context->get_draw_image().view, vk::ImageLayout::eColorAttachmentOptimal
        };
        colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
        colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
        colorAttachment.clearValue = vk::ClearColorValue{ m_clear_color.r, m_clear_color.g, m_clear_color.b, m_clear_color.a };

        vk::RenderingAttachmentInfo depthAttachment{
            m_context->get_depth_image().view, vk::ImageLayout::eDepthStencilAttachmentOptimal
        };
        depthAttachment.loadOp = vk::AttachmentLoadOp::eClear;
        depthAttachment.storeOp = vk::AttachmentStoreOp::eDontCare;
        depthAttachment.clearValue = vk::ClearDepthStencilValue{ 1.0f, 0 };

        vk::RenderingInfo renderingInfo{};
        renderingInfo.renderArea = vk::Rect2D{{0, 0}, m_context->get_swapchain().get_extent()};
        renderingInfo.layerCount = 1;
        renderingInfo.colorAttachmentCount = 1;
        renderingInfo.pColorAttachments = &colorAttachment;
        renderingInfo.pDepthAttachment = &depthAttachment;

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

        uint32_t image_index = m_context->get_swapchain_image_index();
        auto swapchain_image = m_context->get_swapchain().get_images()[image_index];
        auto swapchain_extent = m_context->get_swapchain().get_extent();

        allocated_image& offscreen_draw_image = m_context->get_draw_image();
        transition_image(cmd, offscreen_draw_image.image,
            vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::eTransferSrcOptimal);

        transition_image(cmd, swapchain_image,
            vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);

        copy_image_to_image(cmd, offscreen_draw_image.image, swapchain_image,
            vk::Extent2D{offscreen_draw_image.extent.width, offscreen_draw_image.extent.height}, swapchain_extent);

        transition_image(cmd, swapchain_image, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eColorAttachmentOptimal);
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
