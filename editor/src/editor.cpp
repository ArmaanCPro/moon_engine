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
        ImGuiContext* ctx = ImGui::GetCurrentContext();
        if (!ctx) {
            MOON_ERROR("No ImGui context active!");
            return;
        }

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
