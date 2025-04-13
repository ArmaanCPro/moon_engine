#include <moon.h>

#include <imgui.h>

class sandbox_layer : public moon::layer
{
public:
    sandbox_layer()
        : layer("sandbox")
    {}

    void on_update() override
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

class sandbox_app : public moon::application
{
public:
    sandbox_app()
    {
        MOON_INFO("sandbox app created");
        push_layer(new sandbox_layer());
    }

private:

};

moon::application* moon::create_application()
{
    return new sandbox_app();
}
