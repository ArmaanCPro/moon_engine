#include <moon.h>
#include <moon/core/entry_point.h>

#include "editor_layer.h"

namespace moon
{
    class moon_editor : public moon::application
    {
    public:
        moon_editor()
        {
            MOON_INFO("editor app created");
            push_layer(new editor_layer());
        }

    private:

    };

    moon::application* moon::create_application()
    {
        return new moon_editor();
    }
}
