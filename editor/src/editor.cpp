#include <moon.h>

#include <imgui.h>

class editor_layer : public moon::layer
{
public:
    editor_layer()
        : layer("editor")
    {}

    void on_update() override
    {

    }

    void on_imgui_render() override
    {
        ImGuiContext* engine_context = moon_get_imgui_context();
        if (!engine_context)
        {
            MOON_ERROR("No ImGui context available from engine");
            return;
        }

        ImGui::SetCurrentContext(engine_context);

        ImGui::Begin("hello world");
        ImGui::Text("test");
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
