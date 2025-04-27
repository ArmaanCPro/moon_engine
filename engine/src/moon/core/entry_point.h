#pragma once
#include "application.h"

// in the future, platform specific stuff may exist here. for now, it should be cross-platform compileable
//#ifdef ME_WINDOWS

extern moon::application* moon::create_application();


#ifdef _WIN32

#include "windows.h"
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    moon::log::init();

    auto app = moon::create_application();
    app->run();
    delete app;
}

#else

int main()
{
    moon::log::init();

    MOON_PROFILE_BEGIN_SESSION("Startup", "MoonProfile-Startup.json");
    auto app = moon::create_application();
    MOON_PROFILE_END_SESSION();

    MOON_PROFILE_BEGIN_SESSION("Startup", "MoonProfile-Runtime.json");
    app->run();
    MOON_PROFILE_END_SESSION();

    MOON_PROFILE_BEGIN_SESSION("Startup", "MoonProfile-Shutdown.json");
    delete app;
    MOON_PROFILE_END_SESSION();
    return 0;
}

#endif
