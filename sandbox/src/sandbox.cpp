#include <events/key_event.h>
#include <events/mouse_event.h>
#include <renderer/texture.h>

#include <moon.h>
#include <moon/core/entry_point.h>

#include <imgui.h>

#include <glm/gtc/type_ptr.hpp>

#include "sandbox2d.h"

class sandbox_app : public moon::application
{
public:
    sandbox_app()
    {
        MOON_INFO("sandbox app created");
        push_layer(new sandbox2d_layer());
    }

};

moon::application* moon::create_application()
{
    return new sandbox_app();
}
