#include "moonpch.h"
#include "camera.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace moon
{
    void camera::translate(const glm::vec3& translation)
    {
        position_ += translation;
    }

    void camera::rotate(const glm::quat& rotation)
    {
        rotation_ += rotation;
    }

    void camera::set_position(const glm::vec3& position)
    {
        position_ = position;
    }

    void camera::set_rotation(const glm::quat& rotation)
    {
        rotation_ = rotation;
    }

    glm::mat4 camera::get_view_matrix() const
    {
        glm::mat4 view_matrix = glm::mat4(1.0f);
        view_matrix = glm::translate(view_matrix, position_ * glm::vec3(-1.0f, -1.0f, -1.0f));
        view_matrix = glm::rotate(view_matrix, glm::radians(-1 * rotation_.x), glm::vec3(1.0f, 0.0f, 0.0f));

        return view_matrix;
    }

    glm::mat4 camera::get_projection_matrix() const
    {
        return glm::perspective(glm::radians(fov_), aspect_ratio_, near_plane_, far_plane_);
    }

    glm::mat4 camera::get_view_projection_matrix() const
    {
        return get_projection_matrix() * get_view_matrix();
    }
}
