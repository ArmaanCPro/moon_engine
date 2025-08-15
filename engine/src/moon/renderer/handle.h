#pragma once

#include "core/core.h"

namespace moon
{
    // Non-ref counted handles based on
    // https://enginearchitecture.realtimerendering.com/downloads/reac2023_modern_mobile_rendering_at_hypehype.pdf
    template <class ObjectType>
    class MOON_API handle final
    {
    public:
        handle() = default;
        explicit handle(void* ptr) noexcept
            :
            index_(reinterpret_cast<ptrdiff_t>(ptr) & 0xFFFFFFFF)
            , gen_(reinterpret_cast<ptrdiff_t>(ptr) >> 32)
        {}

        [[nodiscard]] bool empty() const noexcept { return gen_ == 0; }
        [[nodiscard]] bool valid() const noexcept { return gen_ != 0; }
        [[nodiscard]] uint32_t index() const noexcept { return index_; }
        [[nodiscard]] uint32_t gen() const noexcept { return gen_; }
        [[nodiscard]] void* index_as_void() const noexcept { return reinterpret_cast<void*>(static_cast<ptrdiff_t>(index_)); }

        [[nodiscard]] void* handle_as_void() const noexcept
        {
            static_assert(sizeof(void*) >= sizeof(uint64_t));
            return reinterpret_cast<void*>(static_cast<ptrdiff_t>(index) + (static_cast<ptrdiff_t>(gen) << 32));
        }
        [[nodiscard]] bool operator==(const handle<ObjectType> other) const noexcept
        {
            return index_ == other.index_ && gen_ == other.gen_;
        }
        [[nodiscard]] bool operator!=(const handle<ObjectType> other) const noexcept
        {
            return index_ != other.index_ || gen_ != other.gen_;
        }

        [[nodiscard]] explicit operator bool() const noexcept { return gen_ != 0; }

    private:
        handle(uint32_t index, uint32_t gen) noexcept
        : index_(index), gen_(gen) {}

        template <typename ObjectType_, typename ImplObjectType>
        friend class pool;

        uint32_t index_ = 0;
        uint32_t gen_ = 0;
    };

    static_assert(sizeof(handle<class Foo>) == sizeof(uint64_t));

    // specialized dummy structs for type safety
    using compute_pipeline_handle = handle<struct compute_pipeline_tag>;
    using render_pipeline_handle = handle<struct render_pipeline_tag>;
    using raytracing_pipeline_handle = handle<struct raytracing_pipeline_tag>;
    using shader_module_handle = handle<struct shader_module_tag>;
    using sampler_handle = handle<struct sampler_tag>;
    using buffer_handle = handle<struct buffer_tag>;
    using texture_handle = handle<struct texture_tag>;
    using query_pool_handle = handle<struct query_pool_tag>;
    using accel_struct_handle = handle<struct accel_struct_tag>;


    // HOLDER

    // forward declarations (graphics_context.h includes handle so we can't introduce a cyclic include)
    class graphics_context;
    void destroy(graphics_context* ctx, compute_pipeline_handle hdl);
    void destroy(graphics_context* ctx, render_pipeline_handle hdl);
    void destroy(graphics_context* ctx, raytracing_pipeline_handle hdl);
    void destroy(graphics_context* ctx, shader_module_handle hdl);
    void destroy(graphics_context* ctx, sampler_handle hdl);
    void destroy(graphics_context* ctx, buffer_handle hdl);
    void destroy(graphics_context* ctx, texture_handle hdl);
    void destroy(graphics_context* ctx, query_pool_handle hdl);
    void destroy(graphics_context* ctx, accel_struct_handle hdl);

    // helpers to satisfy template requirement
    template<typename T>
    struct is_handle : std::false_type {};

    template<typename ObjectType>
    struct is_handle<handle<ObjectType>> : std::true_type {};

    template<typename T>
    inline constexpr bool is_handle_v = is_handle<T>::value;

    // concept for clearer debug output, could use SFINAE with is_handle_v as well though
    template<typename T>
    concept is_handle_c = is_handle_v<T>;

    // RAII wrapper over handle
    template<typename HandleType>
    requires is_handle_c<HandleType>
    class MOON_API holder final
    {
    public:
        holder() = default;
        holder(graphics_context* ctx, HandleType hdl) noexcept
        :
        ctx_(ctx), hdl_(hdl)
        {}
        ~holder()
        {
            destroy(ctx_, hdl_);
        }
        holder(const holder&) = delete;
        holder(holder&& other) noexcept
            : ctx_(other.ctx_), hdl_(other.hdl_)
        {
            other.ctx_ = nullptr;
            other.hdl_ = HandleType{};
        }
        holder& operator=(const holder&) = delete;
        holder& operator=(holder&& other) noexcept
        {
            std::swap(ctx_, other.ctx_);
            std::swap(hdl_, other.hdl_);
            return *this;
        }
        holder& operator=(std::nullptr_t) noexcept
        {
            this->reset();
            return *this;
        }

        inline operator HandleType() const noexcept { return hdl_; }
        [[nodiscard]] bool empty() const noexcept { return hdl_.empty(); }
        [[nodiscard]] bool valid() const noexcept { return hdl_.valid(); }

        void reset() noexcept
        {
            destroy(ctx_, hdl_);
            ctx_ = nullptr;
            hdl_ = HandleType{};
        }

        HandleType release() noexcept
        {
            ctx_ = nullptr;
            return std::exchange(hdl_, HandleType{});
        }

        uint32_t gen() const noexcept { return hdl_.gen(); }
        uint32_t index() const noexcept { return hdl_.index(); }
        void* index_as_void() const noexcept { return hdl_.index_as_void(); }
        void* handle_as_void() const noexcept { return hdl_.handle_as_void(); }

    private:
        graphics_context* ctx_ = nullptr;
        HandleType hdl_ = {};
    };

}
