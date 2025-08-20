#include "moonpch.h"

#include "vk_context.h"

#include <GLFW/glfw3.h>

#include "core/core.h"
#include "renderer/texture.h"
#include "utils/vk_utils.h"

namespace moon::vulkan
{
    vk_context::vk_context(const native_handle& window)
    {
        m_glfwwindow = std::get<GLFWwindow*>(window);

        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        vkb::InstanceBuilder instanceBuilder;
        auto instRet = instanceBuilder
            .set_app_name("Moon Engine")
            .request_validation_layers(true)
            .require_api_version(1, 3, 0)
            .use_default_debug_messenger()
            .enable_extensions(glfwExtensionCount, glfwExtensions)
            .build();

        vkb::Instance vkbInstance = instRet.value();

        m_instance = vk::UniqueInstance{vkbInstance.instance};
        VULKAN_HPP_DEFAULT_DISPATCHER.init(m_instance.get(),
                                           reinterpret_cast<PFN_vkGetInstanceProcAddr>(glfwGetInstanceProcAddress));
        m_debug_messenger = vkbInstance.debug_messenger;

        glfwCreateWindowSurface(m_instance.get(), m_glfwwindow, nullptr,
                                reinterpret_cast<VkSurfaceKHR*>(&m_surface.get()));

        m_device.init(vkbInstance, m_surface.get(), this);
        VULKAN_HPP_DEFAULT_DISPATCHER.init(m_device.get_device());
        m_swapchain = std::make_unique<vk_swapchain>( *this, m_glfwwindow, m_surface.get(), m_instance.get(),
                                   m_device.get_physical_device(), m_device.get_device() );

        // command buffers
        m_immediate = std::make_unique<vk_immediate_commands>( m_device.get_device(), m_device.get_graphics_queue_index(), "Immediate");

        for (auto i = 0u; i < m_frames.size(); ++i)
        {
            m_frames[i].command_pool = m_device.create_command_pool(m_device.get_graphics_queue_index(), vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
            m_frames[i].command_buffer = m_device.allocate_command_buffer(m_frames[i].command_pool.get());
        }

        // sync structures
        for (auto i = 0u; i < m_frames.size(); ++i)
        {
            m_frames[i].render_fence = m_device.create_fence(true);

            m_frames[i].swapchain_semaphore = m_device.create_semaphore();
            m_frames[i].render_semaphore = m_device.create_semaphore();
        }

        // custom offscreen draw images
        vk::Extent3D extent = vk::Extent3D{m_swapchain->get_extent()};
        m_draw_image = m_device.allocate_image(extent, vk::Format::eR8G8B8A8Unorm,
            vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferSrc |
            vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled);

        m_depth_image = m_device.allocate_image(extent, vk::Format::eD32Sfloat,
            vk::ImageUsageFlagBits::eDepthStencilAttachment
        );
    }

    vk_context::~vk_context()
    {
        m_device.destroy_image(m_depth_image);
        m_device.destroy_image(m_draw_image);
        vkb::destroy_debug_utils_messenger(m_instance.get(), m_debug_messenger);
    }

    vk_command_buffer& vk_context::acquire_command_buffer()
    {
        MOON_CORE_ASSERT_MSG(!m_current_command_buffer.m_context, "Cannot acquire more than 1 command buffer simultaneously!");

#ifdef _M_ARM64
        m_device.get_device().waitIdle(); // temp workaround for windows on snapdragon
#endif

        m_current_command_buffer = vk_command_buffer{this};
        return m_current_command_buffer;
    }

    submit_handle vk_context::submit(command_buffer& cmd, texture_handle present)
    {
        vk_command_buffer& vk_cmd = static_cast<vk_command_buffer&>(cmd);

        MOON_CORE_ASSERT(vk_cmd.m_context != nullptr);
        MOON_CORE_ASSERT(vk_cmd.m_wrapper != nullptr);

        if (present)
        {
            const vulkan_image& tex = *m_textures_pool.get(present);

            MOON_CORE_ASSERT(tex.m_is_swapchain_image);

            tex.transition_layout(vk_cmd.m_wrapper->command_buffer, vk::ImageLayout::ePresentSrcKHR,
                vk::ImageSubresourceRange{ vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});
        }

        const bool should_present = has_swapchain() && present;

        if (should_present)
        {
            const uint64_t signal_value = m_swapchain->get_current_frame_index() + m_swapchain->get_swapchain_image_count();
            m_swapchain->set_timeline_wait_value(m_swapchain->get_current_frame_index(), signal_value);
            m_immediate->signal_semaphore(*m_timeline_semaphore, signal_value);
        }

        vk_cmd.m_last_submit_handle = m_immediate->submit(*vk_cmd.m_wrapper);

        if (should_present)
            m_swapchain->present(m_immediate->acquire_last_submit_semaphore());

        process_deferred_tasks();

        submit_handle hdl = vk_cmd.m_last_submit_handle;

        // reset current command buffer
        m_current_command_buffer = vk_command_buffer{};
        return hdl;
    }

    void vk_context::wait(submit_handle hdl)
    {
        m_immediate->wait(hdl);
    }

    std::expected<holder<buffer_handle>, result> vk_context::create_buffer(const buffer_desc& requested_desc,
        const char* debug_name)
    {
        buffer_desc desc = requested_desc;

        if (debug_name && *debug_name)
        {
            desc.debug_name = debug_name;
        }

        if (!m_use_staging && (desc.storage_type == StorageType::Device))
            desc.storage_type = StorageType::HostVisible;

        vk::BufferUsageFlags usage_flags = ( desc.storage_type == StorageType::Device ) ? vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eTransferSrc
            : vk::BufferUsageFlags{};

        if (desc.usage == 0)
            return std::unexpected(result{result::Code::ArgumentOutOfRange, "Invalid buffer usage!"});

        if (desc.usage & static_cast<uint8_t>(BufferUsageBits::Index))
            usage_flags |= vk::BufferUsageFlagBits::eIndexBuffer;
        if (desc.usage & static_cast<uint8_t>(BufferUsageBits::Vertex))
            usage_flags |= vk::BufferUsageFlagBits::eVertexBuffer;
        if (desc.usage & static_cast<uint8_t>(BufferUsageBits::Uniform))
            usage_flags |= vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eShaderDeviceAddress;
        if (desc.usage & static_cast<uint8_t>(BufferUsageBits::Storage))
            usage_flags |= vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eShaderDeviceAddress;
        if (desc.usage & static_cast<uint8_t>(BufferUsageBits::Indirect))
            usage_flags |= vk::BufferUsageFlagBits::eIndirectBuffer | vk::BufferUsageFlagBits::eShaderDeviceAddress;
        if (desc.usage & static_cast<uint8_t>(BufferUsageBits::ShaderBindingTable))
            usage_flags |= vk::BufferUsageFlagBits::eShaderBindingTableKHR | vk::BufferUsageFlagBits::eShaderDeviceAddress;
        if (desc.usage & static_cast<uint8_t>(BufferUsageBits::AccelStructBuildInputReadOnly))
            usage_flags |= vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR | vk::BufferUsageFlagBits::eShaderDeviceAddress;
        if (desc.usage & static_cast<uint8_t>(BufferUsageBits::AccelStructStorage))
            usage_flags |= vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR | vk::BufferUsageFlagBits::eShaderDeviceAddress;

        MOON_CORE_ASSERT_MSG(usage_flags, "invalid buffer usage!");

        const vk::MemoryPropertyFlags memory_flags = utils::storage_type_to_vk_memory_property_flags(desc.storage_type);

        vulkan_buffer vk_buffer = m_device.allocate_buffer(desc.size, usage_flags, memory_flags, desc.debug_name);
        buffer_handle hdl = m_buffers_pool.create(std::move(vk_buffer));

        if (desc.data)
            upload_buffer(hdl, desc.data, desc.size, 0);

        return holder{ this, hdl };
    }

    std::expected<holder<sampler_handle>, result> vk_context::create_sampler(const sampler_state_desc& desc)
    {
        const vk::SamplerCreateInfo info = utils::sampler_state_desc_to_vk_sampler_create_info(desc, m_device.get_physical_device_properties().limits);

        sampler_handle hdl = m_samplers_pool.create(std::move(m_device.allocate_sampler(info, desc.debug_name)));

        m_awaiting_creation = true;

        return holder{ this, hdl };
    }

    std::expected<holder<texture_handle>, result> vk_context::create_texture(const texture_desc& requested_desc,
        const char* debug_name)
    {
        auto texture = m_device.allocate_texture(requested_desc, debug_name);
        if (!texture)
            return std::unexpected(texture.error());

        texture_handle hdl = m_textures_pool.create(std::move(*texture));

        m_awaiting_creation = true;

        if (requested_desc.data)
        {
            MOON_CORE_ASSERT(requested_desc.type == TextureType::e2D || requested_desc.type == TextureType::eCube);
            MOON_CORE_ASSERT(requested_desc.data_num_mip_levels <= requested_desc.num_mip_levels);
            const uint32_t num_layers = requested_desc.type == TextureType::eCube ? 6 : 1;
            result res = upload_texture(hdl, { .dimensions = requested_desc.dimensions, .num_layers = num_layers, .num_mip_levels = requested_desc.data_num_mip_levels },
                requested_desc.data);
            if (!res.is_ok())
                return std::unexpected(res);
            if (requested_desc.generate_mipmaps)
                this->generate_mipmap(hdl);
        }

        return holder{ this, hdl };
    }

    std::expected<holder<texture_handle>, result> vk_context::create_texture_view(texture_handle hdl,
        const texture_view_desc& desc, const char* debug_name)
    {
        if (!hdl)
            return std::unexpected(result{result::Code::RuntimeError});

        // make a non-owning copy
        vulkan_image image = *m_textures_pool.get(hdl);
        image.m_is_owning_vk_image = false;

        // drop all existing image views as they belong to the base image
        image.m_image_view_storage = vk::ImageView{};
        image.m_image_view_for_framebuffer = {};

        vk::ImageAspectFlags aspect = {};
        if (image.m_is_depth_format)
            aspect = vk::ImageAspectFlagBits::eDepth;
        else if (image.m_is_stencil_format)
            aspect = vk::ImageAspectFlagBits::eStencil;
        else
            aspect = vk::ImageAspectFlagBits::eColor;

        vk::ImageViewType view_type = vk::ImageViewType::e2D;
        switch (desc.type)
        {
        case TextureType::e2D:
            view_type = vk::ImageViewType::e2D;
            break;
        case TextureType::e3D:
            view_type = vk::ImageViewType::e3D;
            break;
        case TextureType::eCube:
            view_type = desc.num_layers > 1 ? vk::ImageViewType::eCubeArray : vk::ImageViewType::eCube;
            break;
        default:
            MOON_CORE_ERROR("Invalid texture type!");
            return std::unexpected(result{result::Code::RuntimeError});
            break;
        }

        const vk::ComponentMapping mapping = {
            static_cast<vk::ComponentSwizzle>(desc.swizzle.r),
            static_cast<vk::ComponentSwizzle>(desc.swizzle.g),
            static_cast<vk::ComponentSwizzle>(desc.swizzle.b),
            static_cast<vk::ComponentSwizzle>(desc.swizzle.a)
        };

        image.m_image_view = image.create_image_view(m_device.get_device(), view_type, image.m_format, aspect,
            desc.mip_level, desc.num_mip_levels, desc.layer, desc.num_layers, mapping, nullptr, debug_name);

        if (image.m_image_view == VK_NULL_HANDLE)
            return std::unexpected(result{result::Code::RuntimeError});

        if (image.m_usage_flags & vk::ImageUsageFlagBits::eStorage)
        {
            if (!desc.swizzle.identity())
            {
                // use identity swizzle for storage images
                image.m_image_view_storage = image.create_image_view(m_device.get_device(), view_type, image.m_format, aspect,
                    desc.mip_level, desc.num_mip_levels, desc.layer, desc.num_layers, {}, nullptr, debug_name);
                MOON_CORE_ASSERT(image.m_image_view_storage != VK_NULL_HANDLE);
            }
        }

        texture_handle return_handle = m_textures_pool.create(std::move(image));

        m_awaiting_creation = true;

        return holder{ this, return_handle };
    }

    std::expected<holder<compute_pipeline_handle>, result> vk_context::create_compute_pipeline(
        const compute_pipeline_desc& desc)
    {
        compute_pipeline_state cps{desc};

        if (desc.spec_info.data && desc.spec_info.data_size)
        {
            // copy into a local storage
            cps.spec_constant_data_storage = malloc(desc.spec_info.data_size);
            memcpy(cps.spec_constant_data_storage, desc.spec_info.data, desc.spec_info.data_size);
            cps.desc.spec_info.data = cps.spec_constant_data_storage;
        }

        return holder{ this, m_compute_pipelines_pool.create(std::move(cps)) };
    }

    std::expected<holder<render_pipeline_handle>, result> vk_context::create_render_pipeline(
        const render_pipeline_desc& desc)
    {
        const bool has_color_attachments = desc.get_num_color_attachments() > 0;
        const bool has_depth_attachment = desc.depth_format != Format::Invalid;
        const bool has_any_attachments = has_color_attachments || has_depth_attachment;

        if (!has_any_attachments)
        {
            MOON_CORE_ERROR("Render pipeline must have at least one attachment!");
            return std::unexpected(result{result::Code::RuntimeError});
        }

        if (desc.sm_mesh.valid())
        {
            if (desc.vertex_input.get_num_attributes() && desc.vertex_input.get_num_input_bindings()) [[unlikely]]
            {
                MOON_CORE_ERROR("Cannot have vertex input with mesh shaders!");
                return std::unexpected(result{result::Code::RuntimeError});
            }
            if (desc.sm_vert.valid()) [[unlikely]]
            {
                MOON_CORE_ERROR("Cannot have vertex shader with mesh shaders!");
                return std::unexpected(result{result::Code::RuntimeError});
            }
            if (desc.sm_tesc.valid() && desc.sm_tese.valid()) [[unlikely]]
            {
                MOON_CORE_ERROR("Cannot have tessellation shaders with mesh shaders!");
                return std::unexpected(result{result::Code::RuntimeError});
            }
            if (desc.sm_geom.valid()) [[unlikely]]
            {
                MOON_CORE_ERROR("Cannot have geometry shader with mesh shaders!");
                return std::unexpected(result{result::Code::RuntimeError});
            }
        }
        else
        {
            if (!desc.sm_vert.valid()) [[unlikely]]
            {
                MOON_CORE_ERROR("Render pipeline must have vertex shader!");
                return std::unexpected(result{result::Code::RuntimeError});
            }
        }

        if (!desc.sm_frag.valid()) [[unlikely]]
        {
            MOON_CORE_ERROR("Render pipeline must have fragment shader!");
            return std::unexpected(result{result::Code::RuntimeError});
        }

        render_pipeline_state rps{desc};

        // iterate and cache vertex input bindings and attributes
        const vertex_input& vstate = rps.desc.vertex_input;

        std::array<bool, vertex_input::s_vertex_buffer_max> bound_buffers = {};
        for (uint32_t i = 0; i < vstate.get_num_input_bindings(); ++i)
        {
            const vertex_input::vertex_attribute& attribute = vstate.attributes[i];

            rps.attributes[i] = vk::VertexInputAttributeDescription{
                attribute.location, attribute.binding, utils::vertex_format_to_vk_format(attribute.format),
                static_cast<uint32_t>(attribute.offset)
            };

            if (!bound_buffers[attribute.binding])
            {
                bound_buffers[attribute.binding] = true;
                rps.bindings[rps.num_bindings++] = vk::VertexInputBindingDescription{
                    attribute.binding, vstate.bindings[attribute.binding].stride, vk::VertexInputRate::eVertex
                };
            }
        }

        if (desc.spec_info.data && desc.spec_info.data_size)
        {
            // copy into local storage
            rps.spec_constant_data_storage = malloc(desc.spec_info.data_size);
            memcpy(rps.spec_constant_data_storage, desc.spec_info.data, desc.spec_info.data_size);
            rps.desc.spec_info.data = rps.spec_constant_data_storage;
        }

        return holder{this, m_render_pipelines_pool.create(std::move(rps))};
    }

    std::expected<holder<raytracing_pipeline_handle>, result> vk_context::create_raytracing_pipeline(
        const ray_tracing_pipeline_desc& desc)
    {
        if (!m_device.has_ray_tracing_pipeline())
        {
            MOON_CORE_ERROR("Raytracing pipeline is not supported on this device!");
            return std::unexpected(result{result::Code::RuntimeError});
        }

        ray_tracing_pipeline_state rtps{desc};

        if (desc.spec_info.data && desc.spec_info.data_size)
        {
            // copy into local storage
            rtps.spec_constant_data_storage = malloc(desc.spec_info.data_size);
            memcpy(rtps.spec_constant_data_storage, desc.spec_info.data, desc.spec_info.data_size);
            rtps.desc.spec_info.data = rtps.spec_constant_data_storage;
        }

        return holder{ this, m_raytracing_pipelines_pool.create(std::move(rtps)) };
    }

    std::expected<holder<shader_module_handle>, result> vk_context::create_shader_module(const shader_module_desc& desc)
    {
        // TODO: desc.data_size ? create_from_spirv : create_from_slang
        MOON_CORE_ASSERT_MSG(desc.data_size, "Shader text parsing (SLANG file) is not yet supported! Please load SPIRV only!");
        auto result = create_shader_module_from_spirv(desc.data, desc.data_size, desc.debug_name);

        if (!result)
            return std::unexpected(result.error());

        return holder{this, m_shader_modules_pool.create(std::move(result.value())) };
    }

    std::expected<holder<query_pool_handle>, result> vk_context::create_query_pool(uint32_t num_queries,
        const char* debug_name)
    {
        const vk::QueryPoolCreateInfo info = {
            {}, vk::QueryType::eTimestamp, num_queries
        };

        vk::QueryPool query_pool;
        VK_CHECK(m_device.get_device().createQueryPool(&info, nullptr, &query_pool));

        if (!query_pool)
            return std::unexpected(result{result::Code::RuntimeError, "Cannot create QueryPool!"});

        VK_CHECK(utils::set_debug_object_name(m_device.get_device(), vk::ObjectType::eQueryPool,
            std::bit_cast<uint64_t>(query_pool), debug_name));

        query_pool_handle hdl = m_queries_pool.create(std::move(query_pool));
        return holder{this, hdl};
    }

    std::expected<holder<accel_struct_handle>, result> vk_context::create_accel_struct(const accel_struct_desc& desc)
    {
        if (!m_device.has_acceleration_structure())
            return std::unexpected(result{result::Code::RuntimeError, "VK_KHR_acceleration_structure not supported on this device!"});

        accel_struct_handle hdl;

        switch (desc.type)
        {
        case AccelStructType::BLAS:
            //hdl = create_blas(desc);
            break;
        case AccelStructType::TLAS:
            //hdl = create_tlas(desc);
            break;
        default:
            MOON_CORE_ERROR("Invalid accel struct type!");
            return std::unexpected(result{result::Code::RuntimeError});
            break;
        }

        if (!hdl)
            return std::unexpected(result{result::Code::RuntimeError});

        m_awaiting_creation = true;

        return holder{this, hdl};
    }

    void vk_context::destroy(compute_pipeline_handle hdl)
    {
        compute_pipeline_state* cps = m_compute_pipelines_pool.get(hdl);
        if (!cps)
            return;

        free(cps->spec_constant_data_storage);

        deferred_task( std::packaged_task<void()>([device = m_device.get_device(), pipeline = cps->pipeline]() {
                device.destroyPipeline(pipeline);
        }));
        deferred_task( std::packaged_task<void()>([device = m_device.get_device(), layout = cps->pipeline_layout]() {
            device.destroyPipelineLayout(layout);
        }));

        m_compute_pipelines_pool.destroy(hdl);
    }

    void vk_context::destroy(render_pipeline_handle hdl)
    {
        render_pipeline_state* rps = m_render_pipelines_pool.get(hdl);
        if (!rps)
            return;

        free(rps->spec_constant_data_storage);

        deferred_task( std::packaged_task<void()>([device = m_device.get_device(), pipeline = rps->pipeline]() {
            device.destroyPipeline(pipeline);
        }));
        deferred_task( std::packaged_task<void()>([device = m_device.get_device(), layout = rps->pipeline_layout]() {
            device.destroyPipelineLayout(layout);
        }));

        m_render_pipelines_pool.destroy(hdl);
    }

    void vk_context::destroy(raytracing_pipeline_handle hdl)
    {
        ray_tracing_pipeline_state* rtps = m_raytracing_pipelines_pool.get(hdl);
        if (!rtps)
            return;

        free(rtps->spec_constant_data_storage);

        deferred_task( std::packaged_task<void()>([device = m_device.get_device(), pipeline = rtps->pipeline]() {
            device.destroyPipeline(pipeline);
        }));
        deferred_task( std::packaged_task<void()>([device = m_device.get_device(), layout = rtps->pipeline_layout]() {
            device.destroyPipelineLayout(layout);
        }));

        m_raytracing_pipelines_pool.destroy(hdl);
    }

    void vk_context::destroy(shader_module_handle hdl)
    {
        shader_module_state* sm = m_shader_modules_pool.get(hdl);
        if (!sm)
            return;

        if (sm->sm != VK_NULL_HANDLE)
        {
            // a shader module can be destroyed while pipelines created using its shaders are still in use
            // https://registry.khronos.org/vulkan/specs/1.3/html/chap9.html#vkDestroyShaderModule
            m_device.get_device().destroyShaderModule(sm->sm);
        }

        m_shader_modules_pool.destroy(hdl);
    }

    void vk_context::destroy(sampler_handle hdl)
    {
        vk::Sampler sampler = *m_samplers_pool.get(hdl);
        m_samplers_pool.destroy(hdl);
        deferred_task( std::packaged_task<void()>([device = m_device.get_device(), sampler]() {
            device.destroySampler(sampler);
        }));
    }

    void vk_context::destroy(buffer_handle hdl)
    {
        vulkan_buffer* buf = m_buffers_pool.get(hdl);
        if (!buf)
            return;

        if (buf->m_mapped_ptr)
            vmaUnmapMemory(m_device.get_allocator(), buf->m_allocation);
        deferred_task(std::packaged_task<void()>(
            [vma = m_device.get_allocator(), buffer = buf->m_buffer, allocation = buf->m_allocation]()
            {
                vmaDestroyBuffer(vma, buffer, allocation);
            }));

        m_buffers_pool.destroy(hdl);
    }

    void vk_context::destroy(texture_handle hdl)
    {
        vulkan_image* tex = m_textures_pool.get(hdl);
        if (!tex)
            return;

        deferred_task(std::packaged_task<void()>(
            [device = m_device.get_device(), image_view = tex->m_image_view]
            {
                device.destroyImageView(image_view);
            }));
        if (tex->m_image_view_storage)
        {
            deferred_task(std::packaged_task<void()>(
                [device = m_device.get_device(), image_view = tex->m_image_view_storage]
                {
                    device.destroyImageView(image_view);
                }));
        }

        for (size_t i = 0; i != s_max_mip_levels; ++i)
        {
            // 6 possible textures for a cubemap
            for (std::size_t j = 0; j != 6; ++j)
            {
                vk::ImageView v = tex->m_image_view_for_framebuffer[i][j];
                if (v != VK_NULL_HANDLE)
                {
                    deferred_task(std::packaged_task<void()>(
                        [device = m_device.get_device(), image_view = v]
                        {
                            device.destroyImageView(image_view);
                        }));
                }
            }
        }

        if (!tex->m_is_owning_vk_image)
            return;

        if (tex->m_mapped_ptr)
            vmaUnmapMemory(m_device.get_allocator(), tex->m_allocation);
        deferred_task(std::packaged_task<void()>(
            [vma = m_device.get_allocator(), image = tex->m_image, allocation = tex->m_allocation]
            {
                vmaDestroyImage(vma, image, allocation);
            }));

        m_textures_pool.destroy(hdl);
        m_awaiting_creation = true; // for validation layers
    }

    void vk_context::destroy(query_pool_handle hdl)
    {
        vk::QueryPool query_pool = *m_queries_pool.get(hdl);
        m_queries_pool.destroy(hdl);
        deferred_task( std::packaged_task<void()>([device = m_device.get_device(), query_pool]() {
            device.destroyQueryPool(query_pool);
        }));
    }

    void vk_context::destroy(accel_struct_handle hdl)
    {
        acceleration_structure* as = m_accel_structs_pool.get(hdl);

        m_accel_structs_pool.destroy(hdl);

        deferred_task( std::packaged_task<void()>([device = m_device.get_device(), as = as->vk_handle]()
        {
            device.destroyAccelerationStructureKHR(as);
        }));
    }

    void vk_context::destroy(framebuffer& fb)
    {
        auto destroy_fb_texture = [this](texture_handle hdl)
        {
            if (!hdl)
                return;
            vulkan_image* tex = m_textures_pool.get(hdl);
            if (!tex || !tex->m_is_owning_vk_image)
                return;
            destroy(hdl);
            hdl = {};
        };

        for (framebuffer::attachment_desc& attachment : fb.color)
        {
            destroy_fb_texture(attachment.texture);
            destroy_fb_texture(attachment.resolve_texture);
        }
        destroy_fb_texture(fb.depth_stencil.texture);
        destroy_fb_texture(fb.depth_stencil.resolve_texture);
    }

    uint64_t vk_context::accel_struct_gpu_address(accel_struct_handle hdl) const
    {
        const acceleration_structure* as = m_accel_structs_pool.get(hdl);
        MOON_CORE_ASSERT(as && as->device_address);
        return as ? static_cast<uint64_t>(as->device_address) : 0u;
    }

    result vk_context::upload_buffer(buffer_handle hdl, const void* data, std::size_t size, std::size_t offset)
    {
        if (!data)
            return result{result::Code::ArgumentOutOfRange};

        MOON_CORE_ASSERT_MSG(size, "Data size should be non-zero");

        vulkan_buffer* buf = m_buffers_pool.get(hdl);
        if (!buf)
            return result{result::Code::ArgumentOutOfRange};

        m_device.buffer_subdata(*buf, offset, size, data);

        return result{};
    }

    result vk_context::download_buffer(buffer_handle hdl, std::size_t size, void* out_data, std::size_t offset)
    {
        if (!out_data)
            return result{result::Code::ArgumentOutOfRange};

        MOON_CORE_ASSERT_MSG(size, "Data size should be non-zero");

        vulkan_buffer* buf = m_buffers_pool.get(hdl);
        if (!buf)
            return result{result::Code::ArgumentOutOfRange};

        buf->get_buffer_subdata(*this, offset, size, out_data);

        return result{};
    }

    uint8_t* vk_context::get_mapped_ptr(buffer_handle hdl) const
    {
        const vulkan_buffer* buf = m_buffers_pool.get(hdl);
        MOON_CORE_ASSERT(buf);
        return buf->is_mapped() ? buf->get_mapped_ptr() : nullptr;
    }

    uint64_t vk_context::gpu_address(buffer_handle hdl, std::size_t offset) const
    {
        MOON_CORE_ASSERT_MSG((offset & 7) == 0, "Buffer offset must by 8 bytes aligned as per GLSL_EXT_buffer_reference spec");
        const vulkan_buffer* buf = m_buffers_pool.get(hdl);
        MOON_CORE_ASSERT(buf && buf->m_device_address);
        return buf ? static_cast<uint64_t>(buf->m_device_address + offset) : 0u;
    }

    void vk_context::flush_mapped_memory(buffer_handle hdl, std::size_t offset, std::size_t size) const
    {
        const vulkan_buffer* buf = m_buffers_pool.get(hdl);
        MOON_CORE_ASSERT(buf);
        buf->flush_mapped_memory(*this, offset, size);
    }

    result vk_context::upload_texture(texture_handle hdl, const texture_range_desc& range, const void* data)
    {
        if (!data)
            return result{result::Code::ArgumentOutOfRange};

        vulkan_image* tex = m_textures_pool.get(hdl);

        if (!tex)
            return result{result::Code::ArgumentOutOfRange};

        [[maybe_unused]] uint32_t num_layers = std::max(range.num_layers, 1u);

        vk::Format vk_format = tex->m_format;

        if (tex->m_type == vk::ImageType::e3D)
        {
            m_device.image_data3D(*tex, vk::Offset3D{range.offset.x, range.offset.y, range.offset.z},
                vk::Extent3D{range.dimensions.width, range.dimensions.height, range.dimensions.depth},
                vk_format, data);
        }
        else
        {
            const vk::Rect2D image_region = {
                vk::Offset2D{ range.offset.x, range.offset.y },
                vk::Extent2D{ range.dimensions.width, range.dimensions.height }
            };
            m_device.image_data2D(*tex, image_region, range.mip_level, range.num_mip_levels, range.layer,
                range.num_layers, vk_format, data);
        }

        return result{};
    }

    result vk_context::download_texture(texture_handle hdl, const texture_range_desc& range, void* out_data)
    {
        if (!out_data)
            return result{result::Code::ArgumentOutOfRange};

        vulkan_image* tex = m_textures_pool.get(hdl);
        MOON_CORE_ASSERT(tex);

        if (!tex)
            return result{result::Code::ArgumentOutOfRange};

        m_device.get_image_data(*tex, vk::Offset3D{range.offset.x, range.offset.y, range.offset.z},
            vk::Extent3D{range.dimensions.width, range.dimensions.height, range.dimensions.depth},
            vk::ImageSubresourceRange{ tex->get_image_aspect_flags(), range.mip_level, range.num_mip_levels, range.layer, range.num_layers },
            tex->m_format, out_data);

        return result{};
    }

    dimensions vk_context::get_dimensions(texture_handle hdl) const
    {
        if (!hdl)
            return {};

        const vulkan_image* tex = m_textures_pool.get(hdl);
        MOON_CORE_ASSERT(tex);
        if (!tex)
            return {};

        return { tex->m_extent.width, tex->m_extent.height, tex->m_extent.depth };
    }

    float vk_context::get_aspect_ratio(texture_handle hdl) const
    {
        if (!hdl)
            return 1.0f;

        const vulkan_image* tex = m_textures_pool.get(hdl);
        MOON_CORE_ASSERT(tex);
        if (!tex)
            return 1.0f;

        return static_cast<float>(tex->m_extent.width) / static_cast<float>(tex->m_extent.height);
    }

    Format vk_context::get_format(texture_handle hdl) const
    {
        if (hdl.empty())
            return Format::Invalid;

        return utils::vk_format_to_format(m_textures_pool.get(hdl)->m_format);
    }

    texture_handle vk_context::get_current_swapchain_texture()
    {
        if (!has_swapchain())
            return {};

        texture_handle hdl = m_swapchain->get_current_texture();
        if (!hdl)
            return {};

        return hdl;
    }

    Format vk_context::get_swapchain_format() const
    {
        if (!has_swapchain())
            return Format::Invalid;

        return utils::vk_format_to_format(m_swapchain->get_format());
    }

    uint32_t vk_context::get_swapchain_image_count() const
    {
        if (!has_swapchain())
            return 0;

        return m_swapchain->get_swapchain_image_count();
    }

    void vk_context::recreate_swapchain(int new_width, int new_height)
    {
    }

    double vk_context::get_timestamp_period_to_ms() const
    {
        return static_cast<double>(m_device.get_physical_device().getProperties().limits.timestampPeriod);
    }

    bool vk_context::get_query_pool_results(query_pool_handle hdl, uint32_t first_query, uint32_t query_count,
        std::size_t stride, void* out_data)
    {
        vk::QueryPool query = *m_queries_pool.get(hdl);
        VK_CHECK(m_device.get_device().getQueryPoolResults(query, first_query, query_count, stride, out_data, stride,
            vk::QueryResultFlagBits::eWait | vk::QueryResultFlagBits::e64));
        return true;
    }

    vk::Pipeline vk_context::get_vk_pipeline(compute_pipeline_handle hdl)
    {
        compute_pipeline_state* cps = m_compute_pipelines_pool.get(hdl);
        if (!cps)
            return VK_NULL_HANDLE;

        check_and_update_descriptor_sets();

        if (cps->last_descriptor_set_layout != m_vk_dsl.get())
        {
            deferred_task(std::packaged_task<void()>([device = m_device.get_device(), pipeline = cps->pipeline]()
            {
                device.destroyPipeline(pipeline);
            }));
            deferred_task(std::packaged_task<void()>([device = m_device.get_device(), layout = cps->pipeline_layout]()
            {
                device.destroyPipelineLayout(layout);
            }));
            cps->pipeline = VK_NULL_HANDLE;
            cps->pipeline_layout = VK_NULL_HANDLE;
            cps->last_descriptor_set_layout = m_vk_dsl.get();
        }

        if (cps->pipeline == VK_NULL_HANDLE)
        {
            const shader_module_state* sm = m_shader_modules_pool.get(cps->desc.sm_comp);
            MOON_CORE_ASSERT(sm);

            std::array<vk::SpecializationMapEntry, specialization_constant_desc::s_specialization_constants_max> entries;
            const vk::SpecializationInfo si_comp = utils::get_pipeline_shader_stage_specialization_info(cps->desc.spec_info, entries.data());

            // create pipeline layout
            {
                // duplicate for MoltenVK
                const std::array dsls = { m_vk_dsl.get(), m_vk_dsl.get(), m_vk_dsl.get(), m_vk_dsl.get() };
                const vk::PushConstantRange range = { vk::ShaderStageFlagBits::eCompute, 0, utils::get_aligned_size(sm->push_constants_size, 16)};
                const vk::PipelineLayoutCreateInfo pli_info = { {}, static_cast<uint32_t>(dsls.size()), dsls.data(), 1, &range };
                VK_CHECK(m_device.get_device().createPipelineLayout(&pli_info, nullptr, &cps->pipeline_layout));
                char pipeline_layout_name[256] = {};
                if (cps->desc.debug_name)
                    std::snprintf(pipeline_layout_name, sizeof(pipeline_layout_name) - 1, "Pipeline Layout: %s", cps->desc.debug_name);
                VK_CHECK(utils::set_debug_object_name(m_device.get_device(), vk::ObjectType::ePipelineLayout, std::bit_cast<uint64_t>(cps->pipeline_layout), pipeline_layout_name));
            }

            const vk::ComputePipelineCreateInfo ci = {
                {}, utils::get_pipeline_shader_stage_create_info(vk::ShaderStageFlagBits::eCompute, sm->sm, cps->desc.entry_point_comp, &si_comp),
                cps->pipeline_layout, VK_NULL_HANDLE, -1
            };
            VK_CHECK(m_device.get_device().createComputePipelines(m_pipeline_cache, 1, &ci, nullptr, &cps->pipeline));
            VK_CHECK(utils::set_debug_object_name(m_device.get_device(), vk::ObjectType::ePipeline, std::bit_cast<uint64_t>(cps->pipeline), cps->desc.debug_name));
        }
        return cps->pipeline;
    }

    vk::Pipeline vk_context::get_vk_pipeline(render_pipeline_handle hdl)
    {
        render_pipeline_state* rps = m_render_pipelines_pool.get(hdl);
        if (!rps)
            return VK_NULL_HANDLE;

        if (rps->last_descriptor_set_layout != m_vk_dsl.get())
        {
            deferred_task(std::packaged_task<void()>([device = m_device.get_device(), pipeline = rps->pipeline]()
            {
                device.destroyPipeline(pipeline);
            }));
            deferred_task(std::packaged_task<void()>([device = m_device.get_device(), layout = rps->pipeline_layout]()
            {
                device.destroyPipelineLayout(layout);
            }));
            rps->pipeline = VK_NULL_HANDLE;
            rps->pipeline_layout = VK_NULL_HANDLE;
            rps->last_descriptor_set_layout = m_vk_dsl.get();
        }
        if (rps->pipeline != VK_NULL_HANDLE)
            return rps->pipeline;

        // build a new vulkan pipeline
        vk::PipelineLayout layout = VK_NULL_HANDLE;
        vk::Pipeline pipeline = VK_NULL_HANDLE;

        const render_pipeline_desc& desc = rps->desc;

        const uint32_t num_color_attachments = rps->desc.get_num_color_attachments();

        // not all attachments are valid, so we need to create color blend attachments for only active attachments
        std::array<vk::PipelineColorBlendAttachmentState, s_max_color_attachments> color_blend_attachment_states;
        std::array<vk::Format, s_max_color_attachments> color_attachments_formats;

        for (uint32_t i = 0; i != num_color_attachments; ++i)
        {
            const color_attachment& attachment = desc.color_attachments[i];
            MOON_CORE_ASSERT(attachment.format != Format::Invalid);
            color_attachments_formats[i] = utils::format_to_vk_format(attachment.format);
            if (!attachment.blend_enabled)
            {
                color_blend_attachment_states[i] = vk::PipelineColorBlendAttachmentState{
                    vk::False, vk::BlendFactor::eOne, vk::BlendFactor::eZero, vk::BlendOp::eAdd,
                    vk::BlendFactor::eOne, vk::BlendFactor::eZero, vk::BlendOp::eAdd,
                    vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
                };
            }
            else
            {
                color_blend_attachment_states[i] = vk::PipelineColorBlendAttachmentState{
                    vk::True, utils::blend_factor_to_vk_blend_factor(attachment.src_rgb_blend_factor),
                    utils::blend_factor_to_vk_blend_factor(attachment.dst_rgb_blend_factor),
                    utils::blend_op_to_vk_blend_op(attachment.rgb_blend_op),
                    utils::blend_factor_to_vk_blend_factor(attachment.src_alpha_blend_factor),
                    utils::blend_factor_to_vk_blend_factor(attachment.dst_alpha_blend_factor),
                    utils::blend_op_to_vk_blend_op(attachment.alpha_blend_op),
                    vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
                };
            }
        }

        const shader_module_state* vert_module = m_shader_modules_pool.get(desc.sm_vert);
        const shader_module_state* tesc_module = m_shader_modules_pool.get(desc.sm_tesc);
        const shader_module_state* tese_module = m_shader_modules_pool.get(desc.sm_tese);
        const shader_module_state* geom_module = m_shader_modules_pool.get(desc.sm_geom);
        const shader_module_state* frag_module = m_shader_modules_pool.get(desc.sm_frag);
        const shader_module_state* task_module = m_shader_modules_pool.get(desc.sm_task);
        const shader_module_state* mesh_module = m_shader_modules_pool.get(desc.sm_mesh);

        MOON_CORE_ASSERT(vert_module || mesh_module);
        MOON_CORE_ASSERT(frag_module);

        if (tesc_module || tese_module || desc.patch_control_points)
        {
            MOON_CORE_ASSERT_MSG(tesc_module && tese_module, "Both tesselation control & evaluation shades should be provided!");
            MOON_CORE_ASSERT(desc.patch_control_points > 0 &&
                desc.patch_control_points <= m_device.get_physical_device().getProperties().limits.maxTessellationPatchSize);
        }

        const vk::PipelineVertexInputStateCreateInfo vertex_input_stateCI = {
            {}, rps->num_attributes, rps->bindings.data(),
            rps->num_attributes, rps->attributes.data()
        };

        std::array<vk::SpecializationMapEntry, specialization_constant_desc::s_specialization_constants_max> entries;
        const vk::SpecializationInfo si = utils::get_pipeline_shader_stage_specialization_info(desc.spec_info, entries.data());

        // create pipeline layout
        {
            rps->shader_stage_flags = {};
            uint32_t push_constants_size = 0;
            const auto update_push_constants_size = [&](const shader_module_state* sm, vk::ShaderStageFlagBits bit)
            {
                if (sm)
                {
                    push_constants_size = std::max(push_constants_size, sm->push_constants_size);
                    rps->shader_stage_flags |= bit;
                }
            };

            update_push_constants_size(vert_module, vk::ShaderStageFlagBits::eVertex);
            update_push_constants_size(tesc_module, vk::ShaderStageFlagBits::eTessellationControl);
            update_push_constants_size(tese_module, vk::ShaderStageFlagBits::eTessellationEvaluation);
            update_push_constants_size(geom_module, vk::ShaderStageFlagBits::eGeometry);
            update_push_constants_size(frag_module, vk::ShaderStageFlagBits::eFragment);
            update_push_constants_size(task_module, vk::ShaderStageFlagBits::eCompute);
            update_push_constants_size(mesh_module, vk::ShaderStageFlagBits::eCompute);

            // maxPushConstantsSize is at least 128 bytes
            const auto& limits = m_device.get_physical_device_properties().limits;
            if (push_constants_size > limits.maxPushConstantsSize)
            {
                MOON_CORE_ERROR("Push constants size is too big {0}! Max size is {1} bytes", push_constants_size, limits.maxPushConstantsSize);
                push_constants_size = limits.maxPushConstantsSize;
            }

            // duplicate for MoltenVK
            const std::array dsls = { m_vk_dsl.get(), m_vk_dsl.get(), m_vk_dsl.get(), m_vk_dsl.get() };
            const vk::PushConstantRange range = { rps->shader_stage_flags, 0, utils::get_aligned_size(push_constants_size, 16) };
            const vk::PipelineLayoutCreateInfo pipeline_layoutCI = { {},
                static_cast<uint32_t>(dsls.size()), dsls.data(), push_constants_size ? 1u : 0u, &range };

            VK_CHECK(m_device.get_device().createPipelineLayout(&pipeline_layoutCI, nullptr, &layout));
            char pipeline_layout_name[256] = {};
            if (desc.debug_name)
                std::snprintf(pipeline_layout_name, sizeof(pipeline_layout_name) - 1, "Pipeline Layout: %s", desc.debug_name);
            VK_CHECK(utils::set_debug_object_name(m_device.get_device(), vk::ObjectType::ePipelineLayout, std::bit_cast<uint64_t>(layout), pipeline_layout_name));
        }

        vk_pipeline_builder builder = {};
        // from Vulkan 1.0
        builder.dynamic_state(vk::DynamicState::eViewport)
            .dynamic_state(vk::DynamicState::eScissor)
            .dynamic_state(vk::DynamicState::eDepthBias)
            .dynamic_state(vk::DynamicState::eBlendConstants)
            // from Vulkan 1.3 or VK_EXT_extended_dynamic_state
            .dynamic_state(vk::DynamicState::eDepthTestEnable)
            .dynamic_state(vk::DynamicState::eDepthWriteEnable)
            .dynamic_state(vk::DynamicState::eDepthCompareOp)
            // from Vulkan 1.3 or VK_EXT_extended_dynamic_state2
            .dynamic_state(vk::DynamicState::eDepthBiasEnable)
            .primitive_topology(utils::topology_to_vk_primitive_topology(desc.topology))
            .rasterization_samples(utils::get_vulkan_samples_count_flags(desc.samples_count, vk::SampleCountFlagBits{m_device.get_framebuffer_msaa_bitmask()}), desc.min_sample_shading)
            .polygon_mode(utils::polygon_mode_to_vk_polygon_mode(desc.polygon_mode))
            .stencil_state_ops(vk::StencilFaceFlagBits::eFront,
                utils::stencil_op_to_vk_stencil_op(desc.front_face_stencil.stencil_failure_op),
                utils::stencil_op_to_vk_stencil_op(desc.front_face_stencil.depth_stencil_pass_op),
                utils::stencil_op_to_vk_stencil_op(desc.front_face_stencil.depth_failure_op),
                utils::compare_op_to_vk_compare_op(desc.front_face_stencil.stencil_compare_op))
            .stencil_masks(vk::StencilFaceFlagBits::eFront, 0xFF, desc.front_face_stencil.write_mask, desc.front_face_stencil.read_mask)
            .stencil_masks(vk::StencilFaceFlagBits::eBack, 0xFF, desc.back_face_stencil.write_mask, desc.back_face_stencil.read_mask)
            .shader_stage(task_module
                ? utils::get_pipeline_shader_stage_create_info(vk::ShaderStageFlagBits::eTaskEXT, task_module->sm, desc.entry_point_task, &si)
                : vk::PipelineShaderStageCreateInfo{})
            .shader_stage(mesh_module
                ? utils::get_pipeline_shader_stage_create_info(vk::ShaderStageFlagBits::eMeshEXT, mesh_module->sm, desc.entry_point_mesh, &si)
                : utils::get_pipeline_shader_stage_create_info(vk::ShaderStageFlagBits::eVertex, vert_module->sm, desc.entry_point_vert, &si))
            .shader_stage(tesc_module
                ? utils::get_pipeline_shader_stage_create_info(vk::ShaderStageFlagBits::eTessellationControl, tesc_module->sm, desc.entry_point_tesc, &si)
                : vk::PipelineShaderStageCreateInfo{})
            .shader_stage(tese_module
                ? utils::get_pipeline_shader_stage_create_info(vk::ShaderStageFlagBits::eTessellationEvaluation, tese_module->sm, desc.entry_point_tese, &si)
                : vk::PipelineShaderStageCreateInfo{})
            .shader_stage(geom_module
                ? utils::get_pipeline_shader_stage_create_info(vk::ShaderStageFlagBits::eGeometry, geom_module->sm, desc.entry_point_geom, &si)
                : vk::PipelineShaderStageCreateInfo{})
            .cull_mode(utils::cull_mode_to_vk_cull_mode(desc.cull_mode))
            .front_face(utils::winding_mode_vk_front_face(desc.winding_mode))
            .vertex_input_state(vertex_input_stateCI)
            .color_attachments(color_blend_attachment_states.data(), color_attachments_formats.data(), (uint32_t)color_blend_attachment_states.size())
            .depth_attachment_format(utils::format_to_vk_format(desc.depth_format))
            .stencil_attachment_format(utils::format_to_vk_format(desc.stencil_format))
            .patch_control_points(desc.patch_control_points);

        pipeline = builder.build(m_device.get_device(), m_pipeline_cache, layout);

        rps->pipeline = pipeline;
        rps->pipeline_layout = layout;

        return pipeline;
    }

    vk::Pipeline vk_context::get_vk_pipeline(raytracing_pipeline_handle hdl)
    {
        // TODO implement raytracing pipeline creation/getting
        return VK_NULL_HANDLE;
    }

    std::expected<shader_module_state, result> vk_context::create_shader_module_from_spirv(const void* spirv,
                                                                                           std::size_t num_bytes, const char* debug_name) const
    {
        vk::ShaderModule sm;

        const vk::ShaderModuleCreateInfo ci = {
            {}, num_bytes, static_cast<const uint32_t*>(spirv)
        };

        {
            const vk::Result res = m_device.get_device().createShaderModule(&ci, nullptr, &sm);

            if (res != vk::Result::eSuccess)
                return std::unexpected(result{result::Code::RuntimeError});
        }

        VK_CHECK(utils::set_debug_object_name(m_device.get_device(), vk::ObjectType::eShaderModule,
            std::bit_cast<uint64_t>(sm), debug_name));
        MOON_CORE_ASSERT(sm != VK_NULL_HANDLE);

        // reflection for push constant size
        SpvReflectShaderModule mdl;
        SpvReflectResult res = spvReflectCreateShaderModule(num_bytes, spirv, &mdl);
        MOON_CORE_ASSERT(res == SPV_REFLECT_RESULT_SUCCESS);

        uint32_t push_constants_size = 0;
        for (uint32_t i = 0; i < mdl.push_constant_block_count; ++i)
        {
            const SpvReflectBlockVariable& block = mdl.push_constant_blocks[i];
            push_constants_size = std::max(push_constants_size, block.size + block.offset);
        }

        spvReflectDestroyShaderModule(&mdl);

        return shader_module_state{ .sm = sm, .push_constants_size = push_constants_size };
    }

    void vk_context::deferred_task(std::packaged_task<void()>&& task, submit_handle hdl) const
    {
        if (hdl.empty())
            hdl = m_immediate->get_next_submit_handle();

        m_deferred_tasks.emplace_back(std::move(task), hdl);
    }

    void vk_context::check_and_update_descriptor_sets()
    {
        if (!m_awaiting_creation)
            return;

        MOON_CORE_ASSERT(m_textures_pool.num_objects() >= 1);
        MOON_CORE_ASSERT(m_samplers_pool.num_objects() >= 1);

        uint32_t new_max_textures = m_current_max_textures;
        uint32_t new_max_samplers = m_current_max_samplers;
        uint32_t new_max_accel_structs = m_current_max_accel_structs;

        while (m_textures_pool.m_objects.size() > new_max_textures)
            new_max_textures *= 2;
        while (m_samplers_pool.m_objects.size() > new_max_samplers)
            new_max_samplers *= 2;
        while (m_accel_structs_pool.m_objects.size() > new_max_accel_structs)
            new_max_accel_structs *= 2;

        if (new_max_textures != m_current_max_textures ||
            new_max_samplers != m_current_max_samplers ||
            m_awaiting_new_immutable_samplers ||
            new_max_accel_structs != m_current_max_accel_structs)
        {
            grow_descriptor_pool(new_max_textures, new_max_samplers, new_max_accel_structs);
        }

        // 1. Sampled and storage images
        std::vector<vk::DescriptorImageInfo> info_sampled_images;
        std::vector<vk::DescriptorImageInfo> info_storage_images;

        info_sampled_images.reserve(m_textures_pool.num_objects());
        info_storage_images.reserve(m_textures_pool.num_objects());

        // use dummy texture to avoid sparse array
        vk::ImageView dummy_image_view = m_textures_pool.m_objects[0].obj_.m_image_view;

        for (const auto& obj : m_textures_pool.m_objects)
        {
            const vulkan_image& img = obj.obj_;
            const vk::ImageView view = obj.obj_.m_image_view;
            const vk::ImageView storage_view = obj.obj_.m_image_view_storage;
            // multisampled images cannot be directly accessed from shaders
            const bool is_texture_available = (img.m_samples & vk::SampleCountFlagBits::e1) != vk::SampleCountFlagBits::e1;
            const bool is_sampled_image = is_texture_available && img.is_sampled_image();
            const bool is_storage_image = is_texture_available && img.is_storage_image();
            info_sampled_images.emplace_back(VK_NULL_HANDLE, is_sampled_image ? view : dummy_image_view, vk::ImageLayout::eShaderReadOnlyOptimal);
            MOON_CORE_ASSERT(info_sampled_images.back().imageView != VK_NULL_HANDLE);
            info_storage_images.emplace_back(VK_NULL_HANDLE, is_storage_image ? storage_view : dummy_image_view, vk::ImageLayout::eGeneral);
        }

        // 2. Samplers
        std::vector<vk::DescriptorImageInfo> info_samplers;
        info_samplers.reserve(m_samplers_pool.num_objects());

        for (const auto& sampler : m_samplers_pool.m_objects)
        {
            info_samplers.emplace_back(sampler.obj_ ? sampler.obj_ : m_samplers_pool.m_objects[0].obj_);
        }

        // 3. Acceleration structures
        std::vector<vk::AccelerationStructureKHR> handles_accel_structs;
        handles_accel_structs.reserve(m_accel_structs_pool.num_objects());
        vk::AccelerationStructureKHR dummy_TLAS = {};
        // use first valid TLAS as a dummy
        for (const auto& as : m_accel_structs_pool.m_objects)
        {
            if (as.obj_.vk_handle && as.obj_.is_tlas)
                dummy_TLAS = as.obj_.vk_handle;
        }
        for (const auto& as : m_accel_structs_pool.m_objects)
            handles_accel_structs.emplace_back(as.obj_.vk_handle ? as.obj_.vk_handle : dummy_TLAS);

        vk::WriteDescriptorSetAccelerationStructureKHR write_accel_struct = {
            static_cast<uint32_t>(handles_accel_structs.size()), handles_accel_structs.data()
        };

        // we have 5 bindings, Textures, Samplers, StorageImages, YUVImages, and AccelerationStructures
        std::array<vk::WriteDescriptorSet, s_num_bindings> writes = {};
        uint32_t num_writes = 0;
        if (!handles_accel_structs.empty())
            writes[num_writes++] = vk::WriteDescriptorSet{
                m_vk_dset.get(), 4u, 0u, static_cast<uint32_t>(handles_accel_structs.size()),
                vk::DescriptorType::eAccelerationStructureKHR, {}, {}, {}, &write_accel_struct
            };

        if (!info_sampled_images.empty())
            writes[num_writes++] = vk::WriteDescriptorSet{
                m_vk_dset.get(), s_texture_binding, 0u, static_cast<uint32_t>(info_sampled_images.size()),
                vk::DescriptorType::eSampledImage, info_sampled_images.data()
            };

        if (!info_samplers.empty())
            writes[num_writes++] = vk::WriteDescriptorSet{
                m_vk_dset.get(), s_num_bindings, 0u, static_cast<uint32_t>(info_samplers.size()),
                vk::DescriptorType::eSampler, info_samplers.data()
            };

        if (!info_storage_images.empty())
            writes[num_writes++] = vk::WriteDescriptorSet{
                m_vk_dset.get(), s_storage_image_binding, 0u, static_cast<uint32_t>(info_storage_images.size()),
                vk::DescriptorType::eStorageImage, info_storage_images.data()
            };

        // do not switch to next descriptor set if there is nothing to update
        if (num_writes)
        {
            m_immediate->wait(m_immediate->get_last_submit_handle());
            m_device.get_device().updateDescriptorSets(num_writes, writes.data(), 0u, nullptr);
        }

        m_awaiting_creation = false;
    }

    void vk_context::bind_default_descriptor_sets(vk::CommandBuffer cmd_buf, vk::PipelineBindPoint bind_point,
        vk::PipelineLayout layout) const
    {
        const std::array dsets = { *m_vk_dset, *m_vk_dset, *m_vk_dset, *m_vk_dset };
        cmd_buf.bindDescriptorSets(bind_point, layout, 0u, static_cast<uint32_t>(dsets.size()), dsets.data(), 0u, nullptr);
    }

    void vk_context::recreate_swapchain()
    {
        m_device.get_device().waitIdle();

        int width, height;
        glfwGetFramebufferSize(m_glfwwindow, &width, &height);

        m_swapchain = std::make_unique<vk_swapchain>(*this, m_glfwwindow, m_surface.get(), m_instance.get(),
                                   m_device.get_physical_device(), m_device.get_device(), vk::PresentModeKHR::eFifo);
        m_resize_requested = false;
    }

    moon::result vk_context::grow_descriptor_pool(uint32_t max_textures, uint32_t max_samplers,
        uint32_t max_accel_structs)
    {
        m_current_max_textures = std::max(m_current_max_textures, max_textures);
        m_current_max_samplers = std::max(m_current_max_samplers, max_samplers);
        m_current_max_accel_structs = std::max(m_current_max_accel_structs, max_accel_structs);

        // TODO: verify not beyond max textures/samplers

        if (m_vk_dsl)
            deferred_task(std::packaged_task<void()>([device = m_device.get_device(), dsl = *m_vk_dsl]()
            {
                device.destroyDescriptorSetLayout(dsl);
            }));
        if (m_vk_dpool)
            deferred_task(std::packaged_task<void()>([device = m_device.get_device(), dpool = *m_vk_dpool]()
            {
                device.destroyDescriptorPool(dpool);
            }));

        std::vector<vk::Sampler> immutable_samplers;
        [[maybe_unused]] const vk::Sampler* immutable_samplers_data = nullptr;

        // create default descriptor set layout which will be shared by the graphics pipeline
        vk::ShaderStageFlags stage_flags = vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eTessellationControl |
            vk::ShaderStageFlagBits::eTessellationEvaluation | vk::ShaderStageFlagBits::eFragment | vk::ShaderStageFlagBits::eCompute;

        if (m_device.has_ray_tracing_pipeline())
            stage_flags |= vk::ShaderStageFlagBits::eRaygenKHR;

        const std::array<vk::DescriptorSetLayoutBinding, s_num_bindings> bindings = {
            utils::get_dsl_binding(s_texture_binding, vk::DescriptorType::eSampledImage, max_textures, stage_flags),
            utils::get_dsl_binding(s_sampler_binding, vk::DescriptorType::eSampler, max_samplers, stage_flags),
            utils::get_dsl_binding(s_storage_image_binding, vk::DescriptorType::eStorageImage, max_textures, stage_flags),
            utils::get_dsl_binding(s_accel_struct_binding, vk::DescriptorType::eAccelerationStructureKHR, max_accel_structs, stage_flags)
        };

        const vk::DescriptorBindingFlags flags = vk::DescriptorBindingFlagBits::eUpdateAfterBind | vk::DescriptorBindingFlagBits::eUpdateUnusedWhilePending |
            vk::DescriptorBindingFlagBits::ePartiallyBound;

        std::array<vk::DescriptorBindingFlags, s_num_bindings> binding_flags = { flags, flags, flags, flags, flags };
        const vk::DescriptorSetLayoutBindingFlagsCreateInfo set_layout_binding_flags_ci = {
            m_device.has_acceleration_structure() ? s_num_bindings : s_num_bindings - 1,
            binding_flags.data()
        };
        const vk::DescriptorSetLayoutCreateInfo dsl_ci = {
            vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPool, static_cast<uint32_t>(bindings.size()), bindings.data(), &set_layout_binding_flags_ci
        };
        VK_CHECK(m_device.get_device().createDescriptorSetLayout(&dsl_ci, nullptr, &m_vk_dsl.get()));
        VK_CHECK(utils::set_debug_object_name(m_device.get_device(), vk::ObjectType::eDescriptorSetLayout,
            std::bit_cast<uint64_t>(m_vk_dsl.get()), "Descriptor Set Layout: vk_context::m_vk_dsl"));

        // create default descriptor pool and allocate 1 descriptor set
        {
            const std::array<vk::DescriptorPoolSize, s_num_bindings> pool_sizes = {
                vk::DescriptorPoolSize{ vk::DescriptorType::eSampledImage, max_textures },
                vk::DescriptorPoolSize{ vk::DescriptorType::eSampler, max_samplers },
                vk::DescriptorPoolSize{ vk::DescriptorType::eStorageImage, max_textures },
                vk::DescriptorPoolSize{ vk::DescriptorType::eCombinedImageSampler, max_textures },
                vk::DescriptorPoolSize{ vk::DescriptorType::eAccelerationStructureKHR, max_accel_structs }
            };
            const vk::DescriptorPoolCreateInfo pool_ci = {
                vk::DescriptorPoolCreateFlagBits::eUpdateAfterBind, 1,
                m_device.has_acceleration_structure() ? s_num_bindings : s_num_bindings - 1,
                pool_sizes.data()
            };
            VK_CHECK(m_device.get_device().createDescriptorPool(&pool_ci, nullptr, &m_vk_dpool.get()));

            const vk::DescriptorSetAllocateInfo ai = {
                m_vk_dpool.get(), 1, &m_vk_dsl.get()
            };
            VK_CHECK(m_device.get_device().allocateDescriptorSets(&ai, &m_vk_dset.get()));
        }

        m_awaiting_new_immutable_samplers = false;
        return result{};
    }

    void vk_context::process_deferred_tasks() const
    {
        while (!m_deferred_tasks.empty() && m_immediate->is_ready(m_deferred_tasks.front().m_handle, true))
        {
            m_deferred_tasks.front().m_task();
            m_deferred_tasks.pop_front();
        }
    }

    void vk_context::wait_deferred_tasks()
    {
        for (auto& task : m_deferred_tasks)
        {
            m_immediate->wait(task.m_handle);
            task.m_task();
        }
        m_deferred_tasks.clear();
    }

    void vk_context::generate_mipmap(texture_handle hdl)
    {
        if (!hdl)
            return;

        const vulkan_image* tex = m_textures_pool.get(hdl);
        if (tex->m_num_levels <= 1)
            return;

        MOON_CORE_ASSERT(tex->m_image_layout != vk::ImageLayout::eUndefined);
        const auto& wrapper = m_immediate->acquire();
        tex->generate_mipmap(wrapper.command_buffer);
        m_immediate->submit(wrapper);
    }
}
