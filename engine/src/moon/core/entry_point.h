#pragma once
#include "application.h"

// in the future, platform specific stuff may exist here. for now, it should be cross-platform compileable
//#ifdef ME_WINDOWS

extern moon::application* moon::create_application();

int main()
{
    moon::log::init();
    auto app = moon::create_application();
    app->run();
    delete app;
    return 0;
}

//#endif
