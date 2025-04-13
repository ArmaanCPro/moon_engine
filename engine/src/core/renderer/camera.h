#pragma once

#include "core/core.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace moon
{
    class MOON_API ortho_camera
    {
    public:
        explicit ortho_camera(float left, float right, float bottom, float top);

        void set_position(const glm::vec3& position) { position_ = position; recalculate_view_matrix(); }
        void set_rotation(float rotation) { rotation_ = rotation; recalculate_view_matrix(); }

        [[nodiscard]] const glm::vec3& get_position() const { return position_; }
        [[nodiscard]] float get_rotation() const { return rotation_; }

        [[nodiscard]] const glm::mat4& get_projection_matrix() const { return projection_matrix_; }
        [[nodiscard]] const glm::mat4& get_view_matrix() const { return view_matrix_; }
        [[nodiscard]] const glm::mat4& get_view_projection_matrix() const { return view_projection_matrix_; }

    private:
        void recalculate_view_matrix();
    private:
        glm::mat4 projection_matrix_;
        glm::mat4 view_matrix_;
        glm::mat4 view_projection_matrix_;

        glm::vec3 position_ {0.0f};
        float rotation_ = 0.0f;
    };

    class MOON_API perspective_camera
    {
    public:
        explicit perspective_camera(const glm::vec3& position = glm::vec3(0.0f), const glm::quat& rotation = glm::vec3(0.0f))
            : position_(position), rotation_(rotation)
        {}

        void translate(const glm::vec3& translation);
        void rotate(const glm::quat& rotation);
        void set_position(const glm::vec3& position);
        void set_rotation(const glm::quat& rotation);

        [[nodiscard]] glm::mat4 get_view_matrix() const;
        [[nodiscard]] glm::mat4 get_projection_matrix() const;
        [[nodiscard]] glm::mat4 get_view_projection_matrix() const;

    private:
        glm::vec3 position_;
        glm::quat rotation_;

        float fov_{45.0f};
        float aspect_ratio_{16.0f/9.0f};
        float near_plane_{0.1f};
        float far_plane_{100.0f};
    };
}
