#pragma once

#include <string>
#include <chrono>
#include <algorithm>
#include <fstream>

#include <thread>

#include "moon/core/core.h"

namespace moon
{
    struct profile_result
    {
        std::string name;
        long long start, end;
        uint32_t thread_id;
    };

    struct instrumentation_session
    {
        std::string name;
    };

    class MOON_API instrumentor
    {
    private:
        instrumentation_session* current_session_;
        std::ofstream output_stream_;
        int profile_count_;
    public:
        instrumentor()
            : current_session_(nullptr), profile_count_(0)
        {
        }

        void begin_session(const std::string& name, const std::string& filepath = "results.json")
        {
            output_stream_.open(filepath);
            write_header();
            current_session_ = new instrumentation_session{ name };
        }

        void end_session()
        {
            write_footer();
            output_stream_.close();
            delete current_session_;
            current_session_ = nullptr;
            profile_count_ = 0;
        }

        void write_profile(const profile_result& result)
        {
            if (profile_count_++ > 0)
                output_stream_ << ",";

            std::string name = result.name;
            std::replace(name.begin(), name.end(), '"', '\'');

            output_stream_ << "{";
            output_stream_ << "\"cat\":\"function\",";
            output_stream_ << "\"dur\":" << (result.end - result.start) << ',';
            output_stream_ << "\"name\":\"" << name << "\",";
            output_stream_ << "\"ph\":\"X\",";
            output_stream_ << "\"pid\":0,";
            output_stream_ << "\"tid\":" << result.thread_id << ",";
            output_stream_ << "\"ts\":" << result.start;
            output_stream_ << "}";

            output_stream_.flush();
        }

        void write_header()
        {
            output_stream_ << "{\"otherData\": {},\"traceEvents\":[";
            output_stream_.flush();
        }

        void write_footer()
        {
            output_stream_ << "]}";
            output_stream_.flush();
        }

        static instrumentor& get()
        {
            static instrumentor instance;
            return instance;
        }
    };

    class MOON_API instrumentation_timer
    {
    public:
        explicit instrumentation_timer(const char* name)
            : name_(name), stopped_(false)
        {
            start_timepoint_ = std::chrono::high_resolution_clock::now();
        }

        ~instrumentation_timer()
        {
            if (!stopped_)
                stop();
        }

        void stop()
        {
            auto endTimepoint = std::chrono::high_resolution_clock::now();

            long long start = std::chrono::time_point_cast<std::chrono::microseconds>(start_timepoint_).time_since_epoch().count();
            long long end = std::chrono::time_point_cast<std::chrono::microseconds>(endTimepoint).time_since_epoch().count();

            size_t thread_id = std::hash<std::thread::id>{}(std::this_thread::get_id());
            instrumentor::get().write_profile({ name_, start, end, (uint32_t)thread_id });

            stopped_ = true;
        }
    private:
        const char* name_;
        std::chrono::time_point<std::chrono::high_resolution_clock> start_timepoint_;
        bool stopped_;
    };
}

#define MOON_PROFILE 0
#if MOON_PROFILE
    #define MOON_PROFILE_BEGIN_SESSION(name, filepath) ::moon::instrumentor::get().begin_session(name, filepath)
    #define MOON_PROFILE_END_SESSION() ::moon::instrumentor::get().end_session()
    #define MOON_PROFILE_SCOPE(name) ::moon::instrumentation_timer CONCATENATE(timer, __LINE__)(name)
    #define CONCATENATE(a, b) CONCATENATE_INNER(a, b)
    #define CONCATENATE_INNER(a, b) a##b
    #define MOON_PROFILE_FUNCTION() MOON_PROFILE_SCOPE(MOON_FUNCSIG)
#else
    #define MOON_PROFILE_BEGIN_SESSION(name, filepath)
    #define MOON_PROFILE_END_SESSION()
    #define MOON_PROFILE_SCOPE(name)
    #define MOON_PROFILE_FUNCTION()
#endif
