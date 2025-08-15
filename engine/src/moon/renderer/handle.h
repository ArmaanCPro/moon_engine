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
        friend class Pool<ObjectType_, ImplObjectType>;

        uint32_t index_ = 0;
        uint32_t gen_ = 0;
    };

}
