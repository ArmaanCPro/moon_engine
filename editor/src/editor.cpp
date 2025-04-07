#include <moon.h>

class editor_app : public moon::application
{
public:
    editor_app()
    {}
    ~editor_app() override
    {}

private:

};

moon::application* moon::create_application()
{
    return new editor_app();
}
