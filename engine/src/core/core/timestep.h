#pragma once

namespace moon
{
    class MOON_API timestep
    {
    public:
        timestep(float time = 0.0f)
            :
            time_(time)
        {}

        [[nodiscard]] float get_seconds() const { return time_; }
        [[nodiscard]] float get_milliseconds() const { return time_ * 1000.0f; }
        [[nodiscard]] float get_microseconds() const { return time_ * 1000000.0f; }
        [[nodiscard]] float get_nanoseconds() const { return time_ * 1000000000.0f; }
        [[nodiscard]] float get_minutes() const { return time_ / 60.0f; }
        [[nodiscard]] float get_hours() const { return time_ / 3600.0f; }

        operator float() const { return time_; }
    private:
        float time_;
    };
}
