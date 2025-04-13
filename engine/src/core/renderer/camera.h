#pragma once

#include "core/core.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace moon
{
    class MOON_API camera
    {
    public:
        virtual ~camera() = default;

        virtual void set_position(const glm::vec3& pos) = 0;
        virtual void set_rotation(const glm::vec3& rot) = 0;

        [[nodiscard]] virtual const glm::vec3& get_position() const = 0;
        [[nodiscard]] virtual const glm::vec3& get_rotation() const = 0;

        [[nodiscard]] virtual const glm::mat4& get_projection_matrix() const = 0;
        [[nodiscard]] virtual const glm::mat4& get_view_matrix() const = 0;
        [[nodiscard]] virtual const glm::mat4& get_view_projection_matrix() const = 0;

    protected:
        camera(const glm::mat4& projection_matrix, const glm::mat4& view_matrix)
            :
            projection_matrix_(projection_matrix),
            view_matrix_(view_matrix)
        {}

        glm::vec3 position_ {0.0f};
        glm::vec3 rotation_ {0.0f};

        glm::mat4 projection_matrix_;
        glm::mat4 view_matrix_;
        glm::mat4 view_projection_matrix_ {1.0f};
    };

    class MOON_API ortho_camera : public camera
    {
    public:
        explicit ortho_camera(float left, float right, float bottom, float top);

        void set_position(const glm::vec3& position) override { position_ = position; recalculate_view_matrix(); }
        void set_rotation(const glm::vec3& rotation) override { rotation_ = rotation; recalculate_view_matrix(); }

        [[nodiscard]] const glm::vec3& get_position() const override { return position_; }
        [[nodiscard]] const glm::vec3& get_rotation() const override { return rotation_; }

        [[nodiscard]] const glm::mat4& get_projection_matrix() const override { return projection_matrix_; }
        [[nodiscard]] const glm::mat4& get_view_matrix() const override { return view_matrix_; }
        [[nodiscard]] const glm::mat4& get_view_projection_matrix() const override { return view_projection_matrix_; }

    private:
        void recalculate_view_matrix();
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
