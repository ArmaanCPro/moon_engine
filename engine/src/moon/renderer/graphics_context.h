#pragma once

#include "command_buffer.h"
#include "device.h"
#include "moon/core/core.h"

#include "handle.h"

#include <expected>

namespace moon
{
    class MOON_API graphics_context
    {
    public:
        virtual ~graphics_context() = default;

        virtual void init() = 0;
        virtual void swap_buffers() = 0;

        virtual void begin_frame() = 0;
        virtual void end_frame() = 0;

        virtual device& get_device() = 0;

        virtual command_buffer& acquire_command_buffer() = 0;

        virtual submit_handle submit(command_buffer& cmd, texture_handle present = {}) = 0;
        virtual void wait(submit_handle hdl = {}); // waiting on empty handle results in vkDeviceWaitIdle()

        [[nodiscard]] virtual std::expected<holder<buffer_handle>, result> create_buffer(const buffer_desc& desc, const char* debug_name = nullptr) = 0;
        [[nodiscard]] virtual std::expected<holder<sampler_handle>, result> create_sampler(const sampler_state_desc& desc) = 0;
        [[nodiscard]] virtual std::expected<holder<texture_handle>, result> create_texture(const texture_desc& desc, const char* debug_name = nullptr) = 0;
        [[nodiscard]] virtual std::expected<holder<texture_handle>, result> create_texture_view(texture_handle hdl,
            const texture_view_desc& desc, const char* debug_name = nullptr);
        [[nodiscard]] virtual std::expected<holder<compute_pipeline_handle>, result> create_compute_pipeline(const compute_pipeline_desc& desc) = 0;
        [[nodiscard]] virtual std::expected<holder<render_pipeline_handle>, result> create_render_pipeline(const render_pipeline_desc& desc) = 0;
        [[nodiscard]] virtual std::expected<holder<raytracing_pipeline_handle>, result> create_raytracing_pipeline(const ray_tracing_pipeline_desc& desc) = 0;
        [[nodiscard]] virtual std::expected<holder<shader_module_handle>, result> create_shader_module(const shader_module_desc& desc) = 0;
        [[nodiscard]] virtual std::expected<holder<query_pool_handle>, result> create_query_pool(uint32_t num_queries, const char* debug_name) = 0;
        [[nodiscard]] virtual std::expected<holder<accel_struct_handle>, result> create_accel_struct(const accel_struct_desc& desc) = 0;

        // handle destruction
        virtual void destroy(compute_pipeline_handle hdl) = 0;
        virtual void destroy(render_pipeline_handle hdl) = 0;
        virtual void destroy(raytracing_pipeline_handle hdl) = 0;
        virtual void destroy(shader_module_handle hdl) = 0;
        virtual void destroy(sampler_handle hdl) = 0;
        virtual void destroy(buffer_handle hdl) = 0;
        virtual void destroy(texture_handle hdl) = 0;
        virtual void destroy(query_pool_handle hdl) = 0;
        virtual void destroy(accel_struct_handle hdl) = 0;

        [[nodiscard]] virtual uint64_t accel_struct_gpu_address(accel_struct_handle hdl) const = 0;

#pragma region buffers
        virtual result upload_buffer(buffer_handle hdl, const void* data, std::size_t size, std::size_t offset = 0) = 0;
        virtual result download_buffer(buffer_handle hdl, std::size_t size, void* out_data, std::size_t offset = 0) = 0;
        [[nodiscard]] virtual uint8_t* get_mapped_ptr(buffer_handle hdl) const = 0;
        [[nodiscard]] virtual uint64_t gpu_address(buffer_handle hdl, std::size_t offset = 0) const = 0;
        virtual void flush_mapped_memory(buffer_handle hdl, std::size_t offset, std::size_t size) const = 0;
#pragma endregion

#pragma region textures
        // 'data' contains mip-levels and layers as in https://registry.khronos.org/KTX/specs/1.0/ktxspec.v1.html
        virtual result upload_texture(texture_handle hdl, const texture_range_desc& range, const void* data) = 0;
        virtual result download_texture(texture_handle hdl, const texture_range_desc& range, void* out_data) = 0;
        [[nodiscard]] virtual dimensions get_dimensions(texture_handle hdl) const = 0;
        [[nodiscard]] virtual float get_aspect_ratio(texture_handle hdl) const = 0;
        [[nodiscard]] virtual Format get_format(texture_handle hdl) const = 0;
#pragma endregion

        [[nodiscard]] virtual texture_handle get_current_swapchain_texture() = 0;
        [[nodiscard]] virtual Format get_swapchain_format() const = 0;
        [[nodiscard]] virtual uint32_t get_swapchain_image_count() const = 0;
        virtual void recreate_swapchain(int new_width, int new_height) = 0;

        // MSAA level is unsupported if ((samples & bitmask) != 0), where samples must be power of 2
        virtual uint32_t get_framebuffer_msaa_bitmask() const = 0;

#pragma region performance queries
        virtual double get_timestamp_period_to_ms() const = 0;
        virtual bool get_query_pool_results( query_pool_handle hdl, uint32_t first_query,
            uint32_t query_count, std::size_t stride, void* out_data) = 0;
#pragma endregion
    };
}
