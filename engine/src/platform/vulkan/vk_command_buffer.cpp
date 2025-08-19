#include "moonpch.h"
#include "vk_command_buffer.h"

#include "utils/vk_utils.h"

#include "vulkan/vk_context.h"

namespace moon::vulkan
{
    vk_immediate_commands::vk_immediate_commands(vk::Device device, uint32_t queue_family_index, const char* debug_name)
        :
        m_device(device), m_queue_family_index(queue_family_index), m_debug_name(debug_name)
    {
        m_device.getQueue(queue_family_index, 0, &m_queue);

        vk::CommandPoolCreateInfo poolCI = {};
        poolCI.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer | vk::CommandPoolCreateFlagBits::eTransient;
        poolCI.queueFamilyIndex = m_queue_family_index;

        VK_CHECK(m_device.createCommandPool(&poolCI, nullptr, &m_command_pool));
        VK_CHECK(moon::vulkan::utils::set_debug_object_name(device, m_command_pool.objectType,
            std::bit_cast<uint64_t>(m_command_pool.operator VkCommandPool()), debug_name));

        const vk::CommandBufferAllocateInfo alloc_info = {
            m_command_pool, vk::CommandBufferLevel::ePrimary, 1
        };

        for (uint32_t i = 0; i != s_max_command_buffers; ++i)
        {
            command_buffer_wrapper& buf = m_buffers[i];
            char fence_name[256] = {};
            char semaphore_name[256] = {};
            if (debug_name)
            {
                snprintf(fence_name, sizeof(fence_name) - 1, "Fence: %s (cmdbuf %u)", debug_name, i);
                snprintf(semaphore_name, sizeof(semaphore_name) - 1, "Semaphore: %s (cmdbuf %u)", debug_name, i);
            }
            buf.semaphore = utils::create_semaphore(m_device, semaphore_name);
            buf.fence = utils::create_fence(m_device, fence_name);
            VK_CHECK(m_device.allocateCommandBuffers(&alloc_info, &buf.command_buffer_allocated));
            m_buffers[i].handle.buffer_index = i;
        }
    }

    vk_immediate_commands::~vk_immediate_commands()
    {
        wait_all();

        for (command_buffer_wrapper& buf : m_buffers)
        {
            m_device.destroyFence(buf.fence);
            m_device.destroySemaphore(buf.semaphore);
        }

        m_device.destroyCommandPool(m_command_pool);
    }

    const vk_immediate_commands::command_buffer_wrapper& vk_immediate_commands::acquire()
    {
        if (!m_num_available_command_buffers)
            purge(); // this should rarely happen

        while (!m_num_available_command_buffers)
        {
            MOON_CORE_TRACE("Waiting for command buffer");
            purge();
        }

        command_buffer_wrapper& current = [&]() -> command_buffer_wrapper&
        {
            for (auto& buf : m_buffers) // TODO consider DOD with separate vectors for available/unvailable command buffers
            {
                if (buf.command_buffer == VK_NULL_HANDLE)
                    return buf;
            }
            return m_buffers[0]; // this should never be hit because we already purged above
        }();

        MOON_CORE_ASSERT_MSG(m_num_available_command_buffers > 0, "No available command buffers");
        MOON_CORE_ASSERT_MSG(current.command_buffer != VK_NULL_HANDLE, "No available command buffers");
        MOON_CORE_ASSERT(current.command_buffer_allocated != VK_NULL_HANDLE);

        current.handle.submit_id = m_submit_counter;
        m_num_available_command_buffers--;

        current.command_buffer = current.command_buffer_allocated;
        current.is_encoding = true;
        constexpr vk::CommandBufferBeginInfo begin_info = { vk::CommandBufferUsageFlagBits::eOneTimeSubmit };
        VK_CHECK(current.command_buffer.begin(&begin_info));

        m_next_submit_handle = current.handle;
        return current;
    }

    submit_handle vk_immediate_commands::submit(const command_buffer_wrapper& wrapper)
    {
        MOON_CORE_ASSERT(wrapper.is_encoding);

        wrapper.command_buffer.end();

        vk::SemaphoreSubmitInfo wait_semaphores[] = { {}, {} };
        uint32_t num_wait_semaphores = 0;
        if (m_wait_semaphore.semaphore)
            wait_semaphores[num_wait_semaphores++] = m_wait_semaphore;
        if (m_last_submit_semaphore.semaphore)
            wait_semaphores[num_wait_semaphores++] = m_last_submit_semaphore;

        vk::SemaphoreSubmitInfo signal_semaphores[] = {
            vk::SemaphoreSubmitInfo{ wrapper.semaphore, 0, vk::PipelineStageFlagBits2::eAllCommands },
            {}
        };
        uint32_t num_signal_semaphores = 1;
        if (m_signal_semaphore.semaphore)
            signal_semaphores[num_signal_semaphores++] = m_signal_semaphore;

        const vk::CommandBufferSubmitInfo buffer_si = { wrapper.command_buffer };
        const vk::SubmitInfo2 submit_info { {}, num_wait_semaphores, wait_semaphores,
            1u, &buffer_si, num_signal_semaphores, signal_semaphores };

        VK_CHECK(m_queue.submit2(1u, &submit_info, wrapper.fence));

        m_last_submit_semaphore.semaphore = wrapper.semaphore;
        m_last_submit_handle = wrapper.handle;
        m_wait_semaphore.semaphore = VK_NULL_HANDLE;
        m_signal_semaphore.semaphore = VK_NULL_HANDLE;

        // reset
        wrapper.is_encoding = false;
        m_submit_counter++;

        if (!m_submit_counter)
        {
            // skip the 0 value - when uint32_t wraps around (null submit_handle)
            m_submit_counter++;
        }

        return m_last_submit_handle;
    }

    void vk_immediate_commands::wait_semaphore(vk::Semaphore semaphore)
    {
        MOON_CORE_ASSERT_MSG(m_wait_semaphore.semaphore == VK_NULL_HANDLE, "Wait semaphore already set");
        m_wait_semaphore.semaphore = semaphore;
    }

    void vk_immediate_commands::signal_semaphore(vk::Semaphore semaphore, uint64_t signal_value)
    {
        MOON_CORE_ASSERT_MSG(m_signal_semaphore.semaphore == VK_NULL_HANDLE, "Signal semaphore already set");
        m_signal_semaphore.semaphore = semaphore;
        m_signal_semaphore.value = signal_value;
    }

    vk::Semaphore vk_immediate_commands::acquire_last_submit_semaphore()
    {
        return std::exchange(m_last_submit_semaphore.semaphore, VK_NULL_HANDLE);
    }

    vk::Fence vk_immediate_commands::get_vk_fence(submit_handle hdl) const
    {
        if (hdl.empty())
            return VK_NULL_HANDLE;

        return m_buffers[hdl.buffer_index].fence;
    }

    submit_handle vk_immediate_commands::get_last_submit_handle() const
    {
        return m_last_submit_handle;
    }

    submit_handle vk_immediate_commands::get_next_submit_handle() const
    {
        return m_next_submit_handle;
    }

    bool vk_immediate_commands::is_ready(submit_handle hdl, bool fast_check_no_vulkan) const
    {
        if (hdl.empty())
            return true; // null handle

        const command_buffer_wrapper& buf = m_buffers[hdl.buffer_index];

        if (buf.command_buffer == VK_NULL_HANDLE)
            return true; // already recycled and not yet reused

        if (buf.handle.submit_id != hdl.submit_id)
            return true; // already recycled and reused by another command buffer

        if (fast_check_no_vulkan)
            return false; // do not ask VulkanAPI about it, just let it retire naturally (when submit_id for this buffer_index gets incremented

        return m_device.waitForFences(1, &buf.fence, vk::True, 0) == vk::Result::eSuccess;
    }

    void vk_immediate_commands::wait(submit_handle hdl)
    {
        if (hdl.empty())
        {
            m_device.waitIdle();
            return;
        }

        if (is_ready(hdl))
            return;

        if (m_buffers[hdl.buffer_index].is_encoding)
        {
            // waiting for a buffer that has not been submitted, probably a logic error
            return;
        }

        VK_CHECK(m_device.waitForFences(1, &m_buffers[hdl.buffer_index].fence, VK_TRUE, std::numeric_limits<uint64_t>::max()));

        purge();
    }

    void vk_immediate_commands::wait_all()
    {
        vk::Fence fences[s_max_command_buffers];
        uint32_t num_fences = 0;

        for (const command_buffer_wrapper& buf : m_buffers)
        {
            if (buf.command_buffer != VK_NULL_HANDLE && !buf.is_encoding)
                fences[num_fences++] = buf.fence;
        }

        if (num_fences)
            VK_CHECK(m_device.waitForFences(num_fences, fences, VK_TRUE, std::numeric_limits<uint64_t>::max()));

        purge();
    }

    void vk_immediate_commands::purge()
    {
        for (uint32_t i = 0; i != m_buffers.size(); ++i)
        {
            // always start checking with oldest submitted buffer, then wrap around
            command_buffer_wrapper& buf = m_buffers[(i + m_last_submit_handle.buffer_index + 1) % m_buffers.size()];

            if (buf.command_buffer == VK_NULL_HANDLE || buf.is_encoding)
            {
                continue;
            }

            if (const vk::Result result = m_device.waitForFences(1, &buf.fence, VK_TRUE, 0);
                result == vk::Result::eSuccess)
            {
                buf.command_buffer.reset();
                VK_CHECK(m_device.resetFences(1, &buf.fence));

                buf.command_buffer = VK_NULL_HANDLE;
                ++m_num_available_command_buffers;
            }
            else if (result != vk::Result::eTimeout)
            {
                VK_CHECK(result);
            }
        }
    }

    vk_command_buffer::vk_command_buffer(vk_context* context)
        :
        m_context(context), m_wrapper(&context->m_immediate->acquire())
    {}

    vk_command_buffer::~vk_command_buffer()
    {
        MOON_CORE_ASSERT_MSG(!m_is_rendering, "Did you forget to call cmdEndRendering()?");
    }

    void vk_command_buffer::transition_to_shader_read_only(texture_handle hdl) const
    {
        const vulkan_image& img = *m_context->m_textures_pool.get(hdl);

        MOON_CORE_ASSERT(!img.m_is_swapchain_image);

        // transition only non-multisampled images - MSAA images cannot be accessed from shaders
        if (img.m_samples == vk::SampleCountFlagBits::e1)
        {
            const vk::ImageAspectFlags flags = img.get_image_aspect_flags();
            // set the result of the previous render pass
            img.transition_layout(m_wrapper->command_buffer,
                img.is_sampled_image() ? vk::ImageLayout::eShaderReadOnlyOptimal : vk::ImageLayout::eGeneral,
                { flags, 0, vk::RemainingMipLevels, 0, vk::RemainingArrayLayers });
        }
    }

    void vk_command_buffer::cmd_bind_ray_tracing_pipeline(raytracing_pipeline_handle hdl)
    {
        if (hdl.empty() || !m_context->get_device().has_ray_tracing_pipeline())
            return;

        m_current_pipeline_graphics = {};
        m_current_pipeline_compute = {};
        m_current_pipeline_raytracing = hdl;

        vk::Pipeline pipeline = m_context->get_vk_pipeline(hdl);

        const ray_tracing_pipeline_state* rtps = m_context->m_raytracing_pipelines_pool.get(hdl);

        MOON_CORE_ASSERT(rtps);
        MOON_CORE_ASSERT(pipeline != VK_NULL_HANDLE);

        if (m_last_pipeline_bound != pipeline)
        {
            m_last_pipeline_bound = pipeline;
            m_wrapper->command_buffer.bindPipeline(vk::PipelineBindPoint::eRayTracingKHR, pipeline);
            m_context->check_and_update_descriptor_sets();
            m_context->bind_default_descriptor_sets(m_wrapper->command_buffer, vk::PipelineBindPoint::eRayTracingKHR, rtps->pipeline_layout);
        }
    }

    void vk_command_buffer::cmd_bind_compute_pipeline(compute_pipeline_handle hdl)
    {
        if (hdl.empty())
            return;

        m_current_pipeline_graphics = {};
        m_current_pipeline_compute = hdl;
        m_current_pipeline_raytracing = {};

        vk::Pipeline pipeline = m_context->get_vk_pipeline(hdl);

        const compute_pipeline_state* cps = m_context->m_compute_pipelines_pool.get(hdl);

        MOON_CORE_ASSERT(cps);
        MOON_CORE_ASSERT(pipeline != VK_NULL_HANDLE);

        if (m_last_pipeline_bound != pipeline)
        {
            m_last_pipeline_bound = pipeline;
            m_wrapper->command_buffer.bindPipeline(vk::PipelineBindPoint::eCompute, pipeline);
            m_context->check_and_update_descriptor_sets();
            m_context->bind_default_descriptor_sets(m_wrapper->command_buffer, vk::PipelineBindPoint::eCompute, cps->pipeline_layout);
        }
    }

    void vk_command_buffer::cmd_dispatch_thread_groups(const dimensions& thread_group_count, const dependencies& deps)
    {
        MOON_CORE_ASSERT_MSG(!m_is_rendering, "You can't be rendering graphics when dispatching for compute!");

        for (uint32_t i = 0; i != dependencies::s_max_submit_dependencies && deps.textures[i]; ++i)
        {
            use_compute_texture(deps.textures[i], vk::PipelineStageFlagBits2::eComputeShader);
        }
        for (uint32_t i = 0; i != dependencies::s_max_submit_dependencies && deps.buffers[i]; ++i)
        {
            const vulkan_buffer* buf = m_context->m_buffers_pool.get(deps.buffers[i]);
            MOON_CORE_ASSERT_MSG(buf->m_usage_flags & vk::BufferUsageFlagBits::eStorageBuffer,
                "Did you forget to specify vk::BufferUsageFlagBits::eStorageBuffer on your buffer?");
            buffer_barrier(deps.buffers[i], vk::PipelineStageFlagBits2::eVertexShader | vk::PipelineStageFlagBits2::eFragmentShader,
                vk::PipelineStageFlagBits2::eComputeShader);
        }

        m_wrapper->command_buffer.dispatch(thread_group_count.width, thread_group_count.height, thread_group_count.depth);
    }

    void vk_command_buffer::cmd_push_debug_group_label(const char* label, uint32_t color_rgba) const
    {
        MOON_CORE_ASSERT(label);
        if (!label)
            return;

        const vk::DebugUtilsLabelEXT utils_label = {
            label, {
                static_cast<float>((color_rgba >> 0) & 0xff) / 255.0f,
                static_cast<float>((color_rgba >> 8) & 0xff) / 255.0f,
                static_cast<float>((color_rgba >> 16) & 0xff) / 255.0f,
                static_cast<float>((color_rgba >> 24) & 0xff) / 255.0f,
            }
        };
        m_wrapper->command_buffer.beginDebugUtilsLabelEXT(utils_label);
    }

    void vk_command_buffer::cmd_insert_debug_event_label(const char* label, uint32_t color_rgba) const
    {
        MOON_CORE_ASSERT(label);
        if (!label)
            return;
        const vk::DebugUtilsLabelEXT utils_label = {
            label, {
                static_cast<float>((color_rgba >> 0) & 0xff) / 255.0f,
                static_cast<float>((color_rgba >> 8) & 0xff) / 255.0f,
                static_cast<float>((color_rgba >> 16) & 0xff) / 255.0f,
                static_cast<float>((color_rgba >> 24) & 0xff) / 255.0f,
            }
        };
        m_wrapper->command_buffer.insertDebugUtilsLabelEXT(utils_label);
    }

    void vk_command_buffer::cmd_pop_debug_group_label() const
    {
        m_wrapper->command_buffer.endDebugUtilsLabelEXT();
    }

    void vk_command_buffer::cmd_begin_rendering(const render_pass& r_pass, const framebuffer& fb,
        const dependencies& deps)
    {
        MOON_CORE_ASSERT_MSG(!m_is_rendering, "Already rendering!");

        m_is_rendering = true;

        for (uint32_t i = 0; i != dependencies::s_max_submit_dependencies && deps.textures[i]; ++i)
        {
            transition_to_shader_read_only(deps.textures[i]);
        }
        for (uint32_t i = 0; i != dependencies::s_max_submit_dependencies && deps.buffers[i]; ++i)
        {
            vk::PipelineStageFlags2 dst_stage_flags = vk::PipelineStageFlagBits2::eVertexShader | vk::PipelineStageFlagBits2::eFragmentShader;

            const vulkan_buffer* buf = m_context->m_buffers_pool.get(deps.buffers[i]);
            MOON_CORE_ASSERT(buf);

            if ((buf->m_usage_flags & vk::BufferUsageFlagBits::eIndexBuffer) || (buf->m_usage_flags & vk::BufferUsageFlagBits::eVertexBuffer))
                dst_stage_flags |= vk::PipelineStageFlagBits2::eVertexInput;
            if (buf->m_usage_flags & vk::BufferUsageFlagBits::eIndirectBuffer)
                dst_stage_flags |= vk::PipelineStageFlagBits2::eDrawIndirect;

            buffer_barrier(deps.buffers[i], vk::PipelineStageFlagBits2::eComputeShader,
                dst_stage_flags);
        }

        const uint32_t num_fb_color_attachments = fb.get_num_color_attachments();
        const uint32_t num_pass_color_attachments = r_pass.get_num_color_attachments();

        MOON_CORE_ASSERT(num_fb_color_attachments == num_pass_color_attachments);

        m_framebuffer = fb;

        // transition all the color attachments
        for (uint32_t i = 0; i != num_fb_color_attachments; ++i)
        {
            if (texture_handle handle = fb.color[i].texture)
            {
                vulkan_image* color_tex = m_context->m_textures_pool.get(handle);
                utils::transition_to_color_attachment(m_wrapper->command_buffer, color_tex);
            }
        }
    }

    void vk_command_buffer::cmd_end_rendering()
    {
        MOON_CORE_ASSERT_MSG(m_is_rendering, "Called cmd_end_rendering while not rendering!");

        m_is_rendering = false;

        m_wrapper->command_buffer.endRendering();

        m_framebuffer = {};
    }

    void vk_command_buffer::cmd_bind_viewport(const viewport& viewport)
    {
        MOON_CORE_ASSERT_MSG(m_is_rendering, "Called cmd_bind_viewport while not rendering!");
        // https://www.saschawillems.de/blog/2019/03/29/flipping-the-vulkan-viewport/
        const vk::Viewport vp = {
            viewport.x, viewport.height - viewport.y, viewport.width, -viewport.height,
            viewport.min_depth, viewport.max_depth
        };
        m_wrapper->command_buffer.setViewport(0, 1, &vp);
    }

    void vk_command_buffer::cmd_bind_scissor_rect(const scissor_rect& rect)
    {
        MOON_CORE_ASSERT_MSG(m_is_rendering, "Called cmd_bind_scissor_rect while not rendering!");
        const vk::Rect2D scissor = {
            { static_cast<int32_t>(rect.x), static_cast<int32_t>(rect.y) },
            { rect.width, rect.height }
        };
        m_wrapper->command_buffer.setScissor(0, 1, &scissor);
    }

    void vk_command_buffer::cmd_bind_render_pipeline(render_pipeline_handle hdl)
    {
        if (!hdl)
            return;

        m_current_pipeline_graphics = hdl;
        m_current_pipeline_compute = {};
        m_current_pipeline_raytracing = {};

        const render_pipeline_state* rps = m_context->m_render_pipelines_pool.get(hdl);

        MOON_CORE_ASSERT(rps);

        const bool has_depth_attachment_pipeline = rps->desc.depth_format != Format::Invalid;
        const bool has_depth_attachment_pass = !m_framebuffer.depth_stencil.texture.empty();

        if (has_depth_attachment_pass != has_depth_attachment_pipeline)
            MOON_CORE_ASSERT_MSG(false, "Make sure your render pass and render pipeline both have matching depth attachments!");

        vk::Pipeline pipeline = m_context->get_vk_pipeline(hdl);

        MOON_CORE_ASSERT(pipeline != VK_NULL_HANDLE);

        if (m_last_pipeline_bound != pipeline)
        {
            m_last_pipeline_bound = pipeline;
            m_wrapper->command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);
            m_context->bind_default_descriptor_sets(m_wrapper->command_buffer, vk::PipelineBindPoint::eGraphics, rps->pipeline_layout);
        }
    }

    void vk_command_buffer::cmd_bind_depth_state(const depth_state& state)
    {
        const vk::CompareOp op = utils::compare_op_to_vk_compare_op(state.compare_op);
        m_wrapper->command_buffer.setDepthWriteEnable(state.depth_write_enabled ? vk::True : vk::False);
        m_wrapper->command_buffer.setDepthTestEnable(op != vk::CompareOp::eAlways || state.depth_write_enabled);

        m_wrapper->command_buffer.setDepthCompareOp(op);
    }

    void vk_command_buffer::cmd_bind_vertex_buffer(uint32_t index, buffer_handle buffer, uint64_t buffer_offset)
    {
        if (buffer.empty())
            return;

        const vulkan_buffer* buf = m_context->m_buffers_pool.get(buffer);
        MOON_CORE_ASSERT(buf->m_usage_flags & vk::BufferUsageFlagBits::eVertexBuffer);

        m_wrapper->command_buffer.bindVertexBuffers(index, 1, &buf->m_buffer, &buffer_offset);
    }

    void vk_command_buffer::cmd_bind_index_buffer(buffer_handle index_buffer, IndexFormat index_format,
        uint64_t index_buffer_offset)
    {
        if (index_buffer.empty())
            return;

        vulkan_buffer* buf = m_context->m_buffers_pool.get(index_buffer);
        MOON_CORE_ASSERT(buf->m_usage_flags & vk::BufferUsageFlagBits::eIndexBuffer);

        const vk::IndexType type = utils::index_format_to_vk_index_type(index_format);
        m_wrapper->command_buffer.bindIndexBuffer(buf->m_buffer, index_buffer_offset, type);
    }

    void vk_command_buffer::cmd_push_constants(const void* data, std::size_t size, std::size_t offset)
    {
        MOON_CORE_ASSERT_MSG(size % 4 == 0, "Push constant size must be a multiple of 4!") // VUID-vkCmdPushConstants-size-00369

        // check push constant size is within max size
        const vk::PhysicalDeviceLimits& limits = m_context->get_device().get_physical_device_properties().limits;
        if (size + offset > limits.maxPushConstantsSize)
        {
            MOON_CORE_ASSERT_MSG(false, "Push constants size exceeded {}, (max {} bytes)", size + offset, limits.maxPushConstantsSize);
            return;
        }

        if (!m_current_pipeline_graphics || !m_current_pipeline_compute || !m_current_pipeline_raytracing)
        {
            MOON_CORE_ASSERT_MSG(false, "You must bind a pipeline before pushing constants!");
            return;
        }

        const render_pipeline_state* rps = m_context->m_render_pipelines_pool.get(m_current_pipeline_graphics);
        const compute_pipeline_state* cps = m_context->m_compute_pipelines_pool.get(m_current_pipeline_compute);
        const ray_tracing_pipeline_state* rtps = m_context->m_raytracing_pipelines_pool.get(m_current_pipeline_raytracing);

        MOON_CORE_ASSERT(rps || cps || rtps);

        vk::PipelineLayout layout = rps ? rps->pipeline_layout : cps ? cps->pipeline_layout : rtps->pipeline_layout;
        vk::ShaderStageFlags stage_flags = rps ? rps->shader_stage_flags : cps ? vk::ShaderStageFlagBits::eCompute : rtps->shader_stage_flags;
        m_wrapper->command_buffer.pushConstants(layout, stage_flags, static_cast<uint32_t>(offset), static_cast<uint32_t>(size), data);
    }

    void vk_command_buffer::cmd_fill_buffer(buffer_handle buffer, std::size_t buffer_offset, std::size_t size,
        uint32_t data)
    {
        MOON_CORE_ASSERT(buffer);
        MOON_CORE_ASSERT(size);
        MOON_CORE_ASSERT(size % 4 == 0);
        MOON_CORE_ASSERT(buffer_offset % 4 == 0);

        vulkan_buffer* buf = m_context->m_buffers_pool.get(buffer);

        buffer_barrier(buffer, vk::PipelineStageFlagBits2::eAllCommands, vk::PipelineStageFlagBits2::eTransfer);

        m_wrapper->command_buffer.fillBuffer(buf->m_buffer, buffer_offset, size, data);

        vk::PipelineStageFlags2 dst_stage = vk::PipelineStageFlagBits2::eVertexShader;

        if (buf->m_usage_flags & vk::BufferUsageFlagBits::eIndirectBuffer)
            dst_stage |= vk::PipelineStageFlagBits2::eDrawIndirect;
        if (buf->m_usage_flags & vk::BufferUsageFlagBits::eVertexBuffer)
            dst_stage |= vk::PipelineStageFlagBits2::eVertexInput;

        buffer_barrier(buffer, vk::PipelineStageFlagBits2::eTransfer, dst_stage);
    }

    void vk_command_buffer::cmd_update_buffer(buffer_handle buffer, std::size_t buffer_offset, std::size_t size,
        const void* data)
    {
        MOON_CORE_ASSERT(buffer);
        MOON_CORE_ASSERT(data);
        MOON_CORE_ASSERT(size && size <= 65536);
        MOON_CORE_ASSERT(size % 4 == 0);
        MOON_CORE_ASSERT(buffer_offset % 4 == 0);

        vulkan_buffer* buf = m_context->m_buffers_pool.get(buffer);

        buffer_barrier(buffer, vk::PipelineStageFlagBits2::eAllCommands, vk::PipelineStageFlagBits2::eTransfer);
        m_wrapper->command_buffer.updateBuffer(buf->m_buffer, buffer_offset, size, data);

        vk::PipelineStageFlags2 dst_stage = vk::PipelineStageFlagBits2::eVertexShader;
        if (buf->m_usage_flags & vk::BufferUsageFlagBits::eIndirectBuffer)
            dst_stage |= vk::PipelineStageFlagBits2::eDrawIndirect;
        if (buf->m_usage_flags & vk::BufferUsageFlagBits::eVertexBuffer)
            dst_stage |= vk::PipelineStageFlagBits2::eVertexInput;

        buffer_barrier(buffer, vk::PipelineStageFlagBits2::eTransfer, dst_stage);
    }

    void vk_command_buffer::cmd_draw(uint32_t vertex_count, uint32_t instance_count, uint32_t first_vertex,
        uint32_t base_instance)
    {
        if (vertex_count == 0)
            return;

        m_wrapper->command_buffer.draw(vertex_count, instance_count, first_vertex, base_instance);
    }

    void vk_command_buffer::cmd_draw_indexed(uint32_t index_count, uint32_t instance_count, uint32_t first_index,
        int32_t vertex_offset, uint32_t base_instance)
    {
        if (index_count == 0)
            return;

        m_wrapper->command_buffer.drawIndexed(index_count, instance_count, first_index, vertex_offset, base_instance);
    }

    void vk_command_buffer::cmd_draw_indirect(buffer_handle indirect_buffer, std::size_t offset, uint32_t draw_count,
        uint32_t stride)
    {
        vulkan_buffer* buf_indirect = m_context->m_buffers_pool.get(indirect_buffer);
        MOON_CORE_ASSERT(buf_indirect);

        m_wrapper->command_buffer.drawIndirect(buf_indirect->m_buffer, offset, draw_count, stride ? stride : sizeof(vk::DrawIndirectCommand));
    }

    void vk_command_buffer::cmd_draw_indexed_indirect(buffer_handle indirect_buffer, std::size_t indirect_buffer_offset,
        std::size_t count_buffer_offset, uint32_t draw_count, uint32_t stride)
    {
        vulkan_buffer* buf_indirect = m_context->m_buffers_pool.get(indirect_buffer);
        MOON_CORE_ASSERT(buf_indirect);

        m_wrapper->command_buffer.drawIndexedIndirect(buf_indirect->m_buffer, indirect_buffer_offset, draw_count,
            stride ? stride : sizeof(vk::DrawIndexedIndirectCommand));
    }

    void vk_command_buffer::cmd_draw_indexed_indirect_count(buffer_handle indirect_buffer,
        std::size_t indirect_buffer_offset, buffer_handle count_buffer, std::size_t count_buffer_offset,
        uint32_t max_draw_count, uint32_t stride)
    {
        vulkan_buffer* buf_indirect = m_context->m_buffers_pool.get(indirect_buffer);
        vulkan_buffer* buf_count = m_context->m_buffers_pool.get(count_buffer);
        MOON_CORE_ASSERT(buf_indirect);
        MOON_CORE_ASSERT(buf_count);

        m_wrapper->command_buffer.drawIndexedIndirectCount(buf_indirect->m_buffer, indirect_buffer_offset,
            buf_count->m_buffer, count_buffer_offset, max_draw_count, stride ? stride : sizeof(vk::DrawIndexedIndirectCommand));
    }

    void vk_command_buffer::cmd_draw_mesh_tasks(const dimensions& thread_group_count)
    {
        m_wrapper->command_buffer.drawMeshTasksEXT(thread_group_count.width, thread_group_count.height, thread_group_count.depth);
    }

    void vk_command_buffer::cmd_draw_mesh_tasks_indirect(buffer_handle indirect_buffer,
        std::size_t indirect_buffer_offset, uint32_t draw_count, uint32_t stride)
    {
        vulkan_buffer* buf_indirect = m_context->m_buffers_pool.get(indirect_buffer);
        MOON_CORE_ASSERT(buf_indirect);

        m_wrapper->command_buffer.drawMeshTasksIndirectEXT(buf_indirect->m_buffer, indirect_buffer_offset, draw_count,
            stride ? stride : sizeof(vk::DrawMeshTasksIndirectCommandEXT));
    }

    void vk_command_buffer::cmd_draw_mesh_tasks_indirect_count(buffer_handle indirect_buffer,
        std::size_t indirect_buffer_offset, buffer_handle count_buffer, std::size_t count_buffer_offset,
        uint32_t max_draw_count, uint32_t stride)
    {
        vulkan_buffer* buf_indirect = m_context->m_buffers_pool.get(indirect_buffer);
        vulkan_buffer* buf_count = m_context->m_buffers_pool.get(count_buffer);
        MOON_CORE_ASSERT(buf_indirect);
        MOON_CORE_ASSERT(buf_count);

        m_wrapper->command_buffer.drawMeshTasksIndirectCountEXT(buf_indirect->m_buffer, indirect_buffer_offset,
            buf_count->m_buffer, count_buffer_offset, max_draw_count, stride ? stride : sizeof(vk::DrawMeshTasksIndirectCommandEXT));
    }

    void vk_command_buffer::cmd_trace_rays(uint32_t width, uint32_t height, uint32_t depth, const dependencies& deps)
    {
        ray_tracing_pipeline_state* rtps = m_context->m_raytracing_pipelines_pool.get(m_current_pipeline_raytracing);
        if (!rtps)
            return;

        MOON_CORE_ASSERT(!m_is_rendering);

        for (uint32_t i = 0; i != dependencies::s_max_submit_dependencies && deps.textures[i]; ++i)
            use_compute_texture(deps.textures[i], vk::PipelineStageFlagBits2::eRayTracingShaderKHR);
        for (uint32_t i = 0; i != dependencies::s_max_submit_dependencies && deps.buffers[i]; ++i)
            buffer_barrier(deps.buffers[i], vk::PipelineStageFlagBits2::eVertexInput | vk::PipelineStageFlagBits2::eFragmentShader,
                vk::PipelineStageFlagBits2::eRayTracingShaderKHR);

        m_wrapper->command_buffer.traceRaysKHR(&rtps->sbt_entry_raygen, &rtps->sbt_entry_miss, &rtps->sbt_entry_hit,
            &rtps->sbt_entry_callable, width, height, depth);
    }

    void vk_command_buffer::cmd_set_blend_color(const float color[4])
    {
        m_wrapper->command_buffer.setBlendConstants(color);
    }

    void vk_command_buffer::cmd_set_depth_bias(float constant_factor, float clamp, float slope_factor)
    {
        m_wrapper->command_buffer.setDepthBias(constant_factor, clamp, slope_factor);
    }

    void vk_command_buffer::cmd_set_depth_bias_enable(bool enable)
    {
        m_wrapper->command_buffer.setDepthBiasEnable(enable ? vk::True : vk::False);
    }

    void vk_command_buffer::cmd_reset_query_pool(query_pool_handle pool, uint32_t first_query, uint32_t query_count)
    {
        vk::QueryPool vk_pool = *m_context->m_queries_pool.get(pool);

        m_wrapper->command_buffer.resetQueryPool(vk_pool, first_query, query_count);
    }

    void vk_command_buffer::cmd_write_timestamp(query_pool_handle pool, uint32_t query_index)
    {
        vk::QueryPool vk_pool = *m_context->m_queries_pool.get(pool);

        m_wrapper->command_buffer.writeTimestamp(static_cast<vk::PipelineStageFlagBits>(vk::PipelineStageFlagBits2::eBottomOfPipe),
            vk_pool, query_index);
    }

    void vk_command_buffer::cmd_clear_color_image(texture_handle tex, const ClearColorValue& value,
        const texture_layers& layers)
    {
        static_assert(sizeof(ClearColorValue) == sizeof(vk::ClearColorValue));

        vulkan_image* img = m_context->m_textures_pool.get(tex);

        if (!img)
            return;

        const vk::ImageSubresourceRange range = {
            img->get_image_aspect_flags(), layers.mip_level, vk::RemainingMipLevels,
            layers.layer, layers.num_layers
        };

        utils::image_memory_barrier_2(m_wrapper->command_buffer, img->m_image,
            utils::stage_access{ vk::PipelineStageFlagBits2::eAllCommands, vk::AccessFlagBits2::eMemoryRead | vk::AccessFlagBits2::eMemoryWrite },
            utils::stage_access{ vk::PipelineStageFlagBits2::eTransfer, vk::AccessFlagBits2::eTransferWrite },
            img->m_image_layout,
            vk::ImageLayout::eTransferDstOptimal,
            range);

        m_wrapper->command_buffer.clearColorImage(img->m_image, vk::ImageLayout::eTransferDstOptimal,
            reinterpret_cast<const vk::ClearColorValue*>(&value),
            1,
            &range);
    }

    void vk_command_buffer::cmd_copy_image(texture_handle src, texture_handle dst, const dimensions& extent,
        const offset3D& src_offset, const offset3D& dst_offset, const texture_layers& src_layers,
        const texture_layers& dst_layers)
    {
        vulkan_image* src_img = m_context->m_textures_pool.get(src);
        vulkan_image* dst_img = m_context->m_textures_pool.get(dst);
        MOON_CORE_ASSERT(src_img && dst_img);
        MOON_CORE_ASSERT(src_img->m_num_layers == dst_img->m_num_layers);

        if (!src_img || !dst_img)
            return;

        const vk::ImageSubresourceRange src_range = {
            src_img->get_image_aspect_flags(), src_layers.mip_level, 1,
            src_layers.layer, src_layers.num_layers
        };

        const vk::ImageSubresourceRange dst_range = {
            dst_img->get_image_aspect_flags(), dst_layers.mip_level, 1,
            dst_layers.layer, dst_layers.num_layers
        };

        MOON_CORE_ASSERT(src_img->m_image_layout != vk::ImageLayout::eUndefined);

        const vk::Extent3D dst_extent = dst_img->m_extent;
        const bool covers_full_dst_image = dst_extent.width == extent.width && dst_extent.height == extent.height && dst_extent.depth == extent.depth
            && dst_offset.x == 0 && dst_offset.y == 0 && dst_offset.z == 0;

        MOON_CORE_ASSERT(covers_full_dst_image || dst_img->m_image_layout != vk::ImageLayout::eUndefined);

        utils::image_memory_barrier_2(m_wrapper->command_buffer, src_img->m_image,
            utils::stage_access{ vk::PipelineStageFlagBits2::eAllCommands, vk::AccessFlagBits2::eMemoryRead | vk::AccessFlagBits2::eMemoryWrite },
            utils::stage_access{ vk::PipelineStageFlagBits2::eTransfer, vk::AccessFlagBits2::eTransferRead },
            src_img->m_image_layout,
            vk::ImageLayout::eTransferSrcOptimal,
            src_range);
        utils::image_memory_barrier_2(m_wrapper->command_buffer, dst_img->m_image,
            utils::stage_access{ vk::PipelineStageFlagBits2::eAllCommands, vk::AccessFlagBits2::eMemoryRead | vk::AccessFlagBits2::eMemoryWrite },
            utils::stage_access{ vk::PipelineStageFlagBits2::eTransfer, vk::AccessFlagBits2::eTransferWrite },
            covers_full_dst_image ? vk::ImageLayout::eUndefined : dst_img->m_image_layout,
            vk::ImageLayout::eTransferDstOptimal,
            dst_range);

        const vk::ImageCopy copy_region = {
            { src_img->get_image_aspect_flags(), src_layers.mip_level, src_layers.layer, src_layers.num_layers },
            { src_offset.x, src_offset.y, src_offset.z },
            { dst_img->get_image_aspect_flags(), dst_layers.mip_level, dst_layers.layer, dst_layers.num_layers },
            { dst_offset.x, dst_offset.y, dst_offset.z },
            { extent.width, extent.height, extent.depth }
        };

        const std::array<vk::Offset3D, 2> src_offsets = { vk::Offset3D{}, vk::Offset3D{
                static_cast<int32_t>(src_offset.x + src_img->m_extent.width),
                static_cast<int32_t>(src_offset.y + src_img->m_extent.height),
                static_cast<int32_t>(src_offset.z + src_img->m_extent.depth)
        }};
        const std::array<vk::Offset3D, 2> dst_offsets = { vk::Offset3D{}, vk::Offset3D{
                static_cast<int32_t>(dst_offset.x + dst_extent.width),
                static_cast<int32_t>(dst_offset.y + dst_extent.height),
                static_cast<int32_t>(dst_offset.z + dst_extent.depth)
        }};
        const vk::ImageBlit blit_region = {
            copy_region.srcSubresource,
            src_offsets,
            copy_region.dstSubresource,
            dst_offsets
        };

        const bool is_compatible = utils::get_bytes_per_pixel(src_img->m_format) == utils::get_bytes_per_pixel(dst_img->m_format);

        is_compatible ? m_wrapper->command_buffer.copyImage(src_img->m_image, vk::ImageLayout::eTransferSrcOptimal,
            dst_img->m_image, vk::ImageLayout::eTransferDstOptimal, 1, &copy_region)
                : m_wrapper->command_buffer.blitImage(src_img->m_image, vk::ImageLayout::eTransferSrcOptimal,
                    dst_img->m_image, vk::ImageLayout::eTransferDstOptimal, 1, &blit_region, vk::Filter::eLinear);

        utils::image_memory_barrier_2(m_wrapper->command_buffer, src_img->m_image,
            utils::stage_access{ vk::PipelineStageFlagBits2::eTransfer, vk::AccessFlagBits2::eTransferRead },
            utils::stage_access{ vk::PipelineStageFlagBits2::eAllCommands, vk::AccessFlagBits2::eMemoryRead | vk::AccessFlagBits2::eMemoryWrite },
            vk::ImageLayout::eTransferSrcOptimal,
            src_img->m_image_layout,
            src_range
        );

        // ternary cascade (branch prediction!)
        const vk::ImageLayout new_layout = dst_img->m_image_layout == vk::ImageLayout::eUndefined
                    ? (dst_img->is_attachment()         ? vk::ImageLayout::eAttachmentOptimal
                        : dst_img->is_sampled_image()   ? vk::ImageLayout::eShaderReadOnlyOptimal
                                                        : vk::ImageLayout::eGeneral)
                    : dst_img->m_image_layout;

        utils::image_memory_barrier_2(m_wrapper->command_buffer, dst_img->m_image,
            utils::stage_access{ vk::PipelineStageFlagBits2::eTransfer, vk::AccessFlagBits2::eTransferWrite },
            utils::stage_access{ vk::PipelineStageFlagBits2::eAllCommands, vk::AccessFlagBits2::eMemoryRead | vk::AccessFlagBits2::eMemoryWrite },
            vk::ImageLayout::eTransferDstOptimal, new_layout, dst_range
        );

        dst_img->m_image_layout = new_layout;
    }

    void vk_command_buffer::cmd_generate_mipmap(texture_handle hdl)
    {
        if (!hdl)
            return;

        const vulkan_image* img = m_context->m_textures_pool.get(hdl);

        if (img->m_num_levels <= 1)
            return;
        MOON_CORE_ASSERT(img->m_image_layout != vk::ImageLayout::eUndefined);

        img->generate_mipmap(m_wrapper->command_buffer);
    }

    void vk_command_buffer::cmd_update_tlas(accel_struct_handle hdl, buffer_handle instances_buffer)
    {
        if (!hdl)
            return;

        acceleration_structure* accel = m_context->m_accel_structs_pool.get(hdl);

        const vk::AccelerationStructureGeometryKHR accel_struct_geometry = [&]() -> vk::AccelerationStructureGeometryKHR
        {
            vk::AccelerationStructureGeometryKHR geometry {};
            geometry.geometryType = vk::GeometryTypeKHR::eInstances;
            geometry.geometry.instances.arrayOfPointers = vk::False;
            geometry.geometry.instances.data = m_context->gpu_address(instances_buffer);
            geometry.flags = vk::GeometryFlagBitsKHR::eOpaque;
            return geometry;
        }();

        vk::AccelerationStructureBuildGeometryInfoKHR build_geometry_info{};
        build_geometry_info.type = vk::AccelerationStructureTypeKHR::eTopLevel;
        build_geometry_info.flags = vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace | vk::BuildAccelerationStructureFlagBitsKHR::eAllowUpdate;
        build_geometry_info.geometryCount = 1;
        build_geometry_info.pGeometries = &accel_struct_geometry;

        vk::AccelerationStructureBuildSizesInfoKHR build_sizes_info{};
        m_context->get_device().get_device().getAccelerationStructureBuildSizesKHR(vk::AccelerationStructureBuildTypeKHR::eDevice,
            &build_geometry_info, &accel->build_range_info.primitiveCount, &build_sizes_info);

        if (!accel->scratch_buffer.valid() || m_context->m_buffers_pool.get(accel->scratch_buffer)->m_size < build_sizes_info.buildScratchSize)
        {
            MOON_CORE_INFO("Recreating scratch buffer for TLAS update");
            accel->scratch_buffer = m_context->create_buffer(
                buffer_desc{
                    .usage = static_cast<uint8_t>(BufferUsageBits::Storage),
                    .storage_type = StorageType::Device,
                    .size = build_sizes_info.buildScratchSize,
                    .debug_name = "scratch_buffer"
                },
                nullptr
            ).value();
        }

        build_geometry_info = vk::AccelerationStructureBuildGeometryInfoKHR{};
        build_geometry_info = vk::AccelerationStructureBuildGeometryInfoKHR{
            vk::AccelerationStructureTypeKHR::eTopLevel,
            vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace | vk::BuildAccelerationStructureFlagBitsKHR::eAllowUpdate,
            vk::BuildAccelerationStructureModeKHR::eUpdate,
            accel->vk_handle,
            accel->vk_handle,
            1u,
            &accel_struct_geometry,
            nullptr,
            { m_context->gpu_address(accel->scratch_buffer) }
        };

        const vk::AccelerationStructureBuildRangeInfoKHR* build_structure_range_infos[] = { &accel->build_range_info };

        {
            const std::array<vk::BufferMemoryBarrier2, 2> barriers = {
                vk::BufferMemoryBarrier2{
                    vk::PipelineStageFlagBits2::eRayTracingShaderKHR,
                    vk::AccessFlagBits2::eMemoryRead,
                    vk::PipelineStageFlagBits2::eAccelerationStructureBuildKHR,
                    vk::AccessFlagBits2::eMemoryWrite | vk::AccessFlagBits2::eMemoryWrite,
                    vk::QueueFamilyIgnored, vk::QueueFamilyIgnored,
                    m_context->m_buffers_pool.get(m_context->m_accel_structs_pool.get(hdl)->buffer)->m_buffer,
                    vk::WholeSize
                },
                vk::BufferMemoryBarrier2{
                    vk::PipelineStageFlagBits2::eTransfer | vk::PipelineStageFlagBits2::eHost,
                    vk::AccessFlagBits2::eMemoryWrite,
                    vk::PipelineStageFlagBits2::eAccelerationStructureBuildKHR,
                    vk::AccessFlagBits2::eMemoryRead,
                    vk::QueueFamilyIgnored,
                    vk::QueueFamilyIgnored,
                    m_context->m_buffers_pool.get(accel->scratch_buffer)->m_buffer,
                    vk::WholeSize
                }
            };
            const vk::DependencyInfo dep_info {
                {}, {}, {},
                static_cast<uint32_t>(barriers.size()),
                barriers.data()
            };
            m_wrapper->command_buffer.pipelineBarrier2(dep_info);
        }

        m_wrapper->command_buffer.buildAccelerationStructuresKHR(1, &build_geometry_info, build_structure_range_infos);
        {
            const vk::BufferMemoryBarrier2 barrier = {
                vk::PipelineStageFlagBits2::eAccelerationStructureBuildKHR,
                vk::AccessFlagBits2::eMemoryRead | vk::AccessFlagBits2::eMemoryWrite,
                vk::PipelineStageFlagBits2::eRayTracingShaderKHR,
                vk::AccessFlagBits2::eMemoryRead,
                vk::QueueFamilyIgnored, vk::QueueFamilyIgnored,
                m_context->m_buffers_pool.get(accel->scratch_buffer)->m_buffer,
                0,
                vk::WholeSize
            };
            const vk::DependencyInfo dep_info {
                {}, {}, {},
                1,
                &barrier
            };
            m_wrapper->command_buffer.pipelineBarrier2(dep_info);
        }
    }

    void vk_command_buffer::use_compute_texture(texture_handle hdl, vk::PipelineStageFlags2 dst_stage) const
    {
        MOON_CORE_ASSERT(!hdl.empty());
        vulkan_image& tex = *m_context->m_textures_pool.get(hdl);

        (void)dst_stage; // TODO add extra dst_stage

        if (!tex.is_storage_image() && !tex.is_sampled_image())
        {
            MOON_CORE_ASSERT_MSG(false, "Did you forget to specify TextureUsageBits::Storage or TextureUsageBits::Sampled on your texture?")
            return;
        }
        tex.transition_layout(m_wrapper->command_buffer,
            tex.is_storage_image() ? vk::ImageLayout::eGeneral : vk::ImageLayout::eShaderReadOnlyOptimal,
            { tex.get_image_aspect_flags(), 0, vk::RemainingMipLevels, 0, vk::RemainingArrayLayers} );
    }

    void vk_command_buffer::buffer_barrier(buffer_handle handle, vk::PipelineStageFlags2 src_stage,
        vk::PipelineStageFlags2 dst_stage) const
    {
        vulkan_buffer* buf = m_context->m_buffers_pool.get(handle);

        vk::BufferMemoryBarrier2 barrier = {
            src_stage, {}, dst_stage, {},
            vk::QueueFamilyIgnored, vk::QueueFamilyIgnored,
            buf->m_buffer, 0, vk::WholeSize
        };

        if (src_stage & vk::PipelineStageFlagBits2::eTransfer)
            barrier.srcAccessMask |= vk::AccessFlagBits2::eTransferWrite | vk::AccessFlagBits2::eTransferRead;
        else
            barrier.srcAccessMask |= vk::AccessFlagBits2::eShaderWrite | vk::AccessFlagBits2::eShaderRead;
        if (dst_stage & vk::PipelineStageFlagBits2::eTransfer)
            barrier.dstAccessMask |= vk::AccessFlagBits2::eTransferWrite | vk::AccessFlagBits2::eTransferRead;
        else
            barrier.dstAccessMask |= vk::AccessFlagBits2::eShaderWrite | vk::AccessFlagBits2::eShaderRead;

        const vk::DependencyInfo dep_info = {
            {}, 0, nullptr, 1, &barrier
        };

        m_wrapper->command_buffer.pipelineBarrier2(&dep_info);
    }
}
