#include "moonpch.h"
#include "imgui_layer.h"
#include <imgui.h>

//#ifndef MOON_IS_MONOLITHIC
// declared in moon.h, this function is to allow editor to access our context, useful for dll linking.
extern "C" MOON_API ImGuiContext* moon_get_imgui_context()
{
    MOON_PROFILE_FUNCTION();

    return ImGui::GetCurrentContext();
}
//#endif
