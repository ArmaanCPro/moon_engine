#pragma once

#include "window.h"
#include "core/core.h"

namespace moon
{
    class MOON_API application
    {
    public:
        application();
        virtual ~application();

        void run();

    private:
        std::unique_ptr<window> window_;
        bool running_ = true;
    };

    // to be defined in the client
    application* create_application();
}
