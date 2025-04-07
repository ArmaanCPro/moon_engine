#pragma once
#include "application.h"

#ifdef ME_WINDOWS

extern moon::application* moon::create_application();

int main()
{
    auto app = moon::create_application();
    app->run();
    delete app;
    return 0;
}

#endif
