#pragma once

#include "core/core.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace moon
{
    class MOON_API camera
    {
    public:
        explicit camera(const glm::vec3& position = glm::vec3(0.0f), const glm::quat& rotation = glm::vec3(0.0f))
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
