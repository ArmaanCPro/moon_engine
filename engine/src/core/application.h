#pragma once

#include "core/core.h"

namespace moon
{
    class MOON_API application
    {
    public:
        application();
        virtual ~application();

        void run();
    };

    // to be defined in the client
    application* create_application();
}
