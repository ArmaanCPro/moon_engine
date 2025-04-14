#pragma once

#include "renderer/camera.h"
#include "core/timestep.h"

#include "moon/events/event.h"
#include "events/application_event.h"
#include "moon/events/mouse_event.h"

#include <glm/glm.hpp>

namespace moon
{
    class MOON_API orthographic_camera_controller
    {
    public:
        orthographic_camera_controller(float aspect_ratio, bool rotation = false);

        void on_update(timestep ts);
        void on_event(event& e);

        ortho_camera& get_camera() { return camera_; }
        const ortho_camera& get_camera() const { return camera_; }
    private:
        float aspect_ratio_;
        float zoom_level_ {1.0f};
        ortho_camera camera_;

        bool rotation_;
        glm::vec3 camera_position_{0.0f};
        float camera_rotation_{0.0f};
        float camera_translation_speed_{2.0f};
        float camera_rotation_speed_{180.0f};
    };
}
