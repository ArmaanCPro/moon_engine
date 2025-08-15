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

        vk::CommandPoolCreateInfo poolCI = init::command_pool_create_info();
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

        MOON_CORE_ASSERT(m_num_available_command_buffers > 0, "No available command buffers");
        MOON_CORE_ASSERT(current.command_buffer != VK_NULL_HANDLE, "No available command buffers");
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
        MOON_CORE_ASSERT(m_wait_semaphore.semaphore == VK_NULL_HANDLE, "Wait semaphore already set");
        m_wait_semaphore.semaphore = semaphore;
    }

    void vk_immediate_commands::signal_semaphore(vk::Semaphore semaphore, uint64_t signal_value)
    {
        MOON_CORE_ASSERT(m_signal_semaphore.semaphore == VK_NULL_HANDLE, "Signal semaphore already set");
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
        m_context(context), m_wrapper(&context->m_immediate_commands.acquire())
    {}

    vk_command_buffer::~vk_command_buffer()
    {
        MOON_CORE_ASSERT(!m_is_rendering, "Did you forget to call cmdEndRendering()?");
    }

    void vk_command_buffer::transition_to_shader_read_only(texture_handle surface) const
    {

    }
}
