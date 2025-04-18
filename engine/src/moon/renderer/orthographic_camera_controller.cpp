#include "moonpch.h"
#include "orthographic_camera_controller.h"

#include "moon/core/input.h"
#include "moon/core/key_codes.h"
#include "moon/events/event.h"
#include "moon/events/application_event.h"
#include "moon/events/mouse_event.h"

namespace moon
{
    orthographic_camera_controller::orthographic_camera_controller(float aspect_ratio, bool rotation)
        :
        aspect_ratio_(aspect_ratio),
        camera_(-aspect_ratio * zoom_level_, aspect_ratio * zoom_level_, -zoom_level_, zoom_level_),
        rotation_(rotation)
    {}

    void orthographic_camera_controller::on_update(timestep ts)
    {
        MOON_PROFILE_FUNCTION();

        if (input::is_key_pressed(MOON_KEY_W))
            camera_position_.y += camera_translation_speed_ * ts;
        if (input::is_key_pressed(MOON_KEY_S))
            camera_position_.y -= camera_translation_speed_ * ts;

        if (input::is_key_pressed(MOON_KEY_A))
            camera_position_.x -= camera_translation_speed_ * ts;
        if (input::is_key_pressed(MOON_KEY_D))
            camera_position_.x += camera_translation_speed_ * ts;

        if (rotation_)
        {
            if (input::is_key_pressed(MOON_KEY_Q))
                camera_rotation_ += camera_rotation_speed_ * ts;
            if (input::is_key_pressed(MOON_KEY_E))
                camera_rotation_ -= camera_rotation_speed_ * ts;

            if (camera_rotation_ > 180.0f)
                camera_rotation_ -= 360.0f;
            else if (camera_rotation_ <= -180.0f)
                camera_rotation_ += 360.0f;

            camera_.set_rotation(camera_rotation_);
        }

        camera_.set_position(camera_position_);

        camera_translation_speed_ = zoom_level_;
    }

    void orthographic_camera_controller::on_event(event& e)
    {
        MOON_PROFILE_FUNCTION();

        event_dispatcher dispatcher(e);
        dispatcher.dispatch<mouse_scrolled_event>([&](mouse_scrolled_event& me) -> bool
        {
            zoom_level_ -= me.get_y_offset() * 0.25f;
            zoom_level_ = std::max(zoom_level_, 0.25f);
            camera_.set_projection(-aspect_ratio_ * zoom_level_, aspect_ratio_ * zoom_level_, -zoom_level_, zoom_level_);
            return false;
        });
        dispatcher.dispatch<window_resize_event>([&](window_resize_event& we) -> bool
        {
            aspect_ratio_ = (float)we.get_width() / (float)we.get_height();
            camera_.set_projection(-aspect_ratio_ * zoom_level_, aspect_ratio_ * zoom_level_, -zoom_level_, zoom_level_);
            return false;
        });
    }
}
