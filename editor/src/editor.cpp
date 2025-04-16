#include <moon.h>
#include <moon/core/entry_point.h>

#include <imgui.h>

class editor_layer : public moon::layer
{
public:
    editor_layer()
        : layer("editor")
    {}

    void on_update(moon::timestep ts) override
    {

    }

    void on_imgui_render() override
    {
        ImGui::Begin("test");
        ImGui::Text("Hello, world!");
        ImGui::End();
    }

    void on_event(moon::event&) override
    {

    }
};

class editor_app : public moon::application
{
public:
    editor_app()
    {
        MOON_INFO("editor app created");
        push_layer(new editor_layer());
    }

private:

};

moon::application* moon::create_application()
{
    return new editor_app();
}
