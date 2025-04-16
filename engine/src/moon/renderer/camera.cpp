#include "moonpch.h"
#include "camera.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace moon
{
    ortho_camera::ortho_camera(float left, float right, float bottom, float top)
        :
        projection_matrix_(glm::ortho(left, right, bottom, top, -1.0f, 1.0f)),
        view_matrix_(1.0f)
    {
        MOON_PROFILE_FUNCTION();

        view_projection_matrix_ = projection_matrix_ * view_matrix_;
    }

    void ortho_camera::set_projection(float left, float right, float bottom, float top)
    {
        MOON_PROFILE_FUNCTION();

        projection_matrix_ = glm::ortho(left, right, bottom, top, -1.0f, 1.0f);
        view_projection_matrix_ = projection_matrix_ * view_matrix_;
    }

    void ortho_camera::recalculate_view_matrix()
    {
        MOON_PROFILE_FUNCTION();

        glm::mat4 transform = glm::rotate(glm::mat4(1.0f), glm::radians(rotation_), glm::vec3(0, 0, 1))
            * glm::translate(glm::mat4(1.0f), position_);

        view_matrix_ = glm::inverse(transform);
        view_projection_matrix_ = projection_matrix_ * view_matrix_;
    }

    void perspective_camera::translate(const glm::vec3& translation)
    {
        position_ += translation;
    }

    void perspective_camera::rotate(const glm::quat& rotation)
    {
        rotation_ += rotation;
    }

    void perspective_camera::set_position(const glm::vec3& position)
    {
        position_ = position;
    }

    void perspective_camera::set_rotation(const glm::quat& rotation)
    {
        rotation_ = rotation;
    }

    glm::mat4 perspective_camera::get_view_matrix() const
    {
        glm::mat4 view_matrix = glm::mat4(1.0f);
        view_matrix = glm::translate(view_matrix, position_ * glm::vec3(-1.0f, -1.0f, -1.0f));
        view_matrix = glm::rotate(view_matrix, glm::radians(-1 * rotation_.x), glm::vec3(1.0f, 0.0f, 0.0f));

        return view_matrix;
    }

    glm::mat4 perspective_camera::get_projection_matrix() const
    {
        return glm::perspective(glm::radians(fov_), aspect_ratio_, near_plane_, far_plane_);
    }

    glm::mat4 perspective_camera::get_view_projection_matrix() const
    {
        return get_projection_matrix() * get_view_matrix();
    }
}
