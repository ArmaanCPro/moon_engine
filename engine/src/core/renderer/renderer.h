#pragma  once

namespace moon
{
    enum class RendererAPI
    {
        None = 0, OpenGL = 1,
    };

    class MOON_API renderer
    {
    public:

        inline static RendererAPI get_api() { return renderer_api; }
    private:
        static RendererAPI renderer_api;
    };
}
