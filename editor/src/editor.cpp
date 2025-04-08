#include <moon.h>

class editor_layer : public moon::layer
{
public:
    editor_layer()
        : layer("editor")
    {}

    void on_update() override
    {
        //MOON_INFO("editor layer update");
    }

    void on_event(moon::event& e) override
    {
        MOON_TRACE("{0}", e.to_string());
    }
};

class editor_app : public moon::application
{
public:
    editor_app()
    {
        MOON_INFO("editor app created");
        push_layer(new editor_layer());
        push_overlay(new moon::imgui_layer());
    }

private:

};

moon::application* moon::create_application()
{
    return new editor_app();
}
