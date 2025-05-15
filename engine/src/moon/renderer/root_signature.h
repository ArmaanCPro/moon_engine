#pragma once

#include "binding_set.h"
#include "moon/core/core.h"

namespace moon
{
    class MOON_API root_signature_desc
    {
    public:
        virtual ~root_signature_desc() = default;

        virtual const void* get_native_desc() const = 0;
        static ref<root_signature_desc> create(const binding_layout& layout);
    };

    class MOON_API root_signature
    {
    public:
        virtual ~root_signature() = default;

        virtual void* get_native_handle() const = 0;

        static ref<root_signature> create(const root_signature_desc& desc);
    };
}
