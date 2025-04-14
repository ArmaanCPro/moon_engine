#include "layer.h"

#include <utility>

namespace moon
{
    layer::layer(std::string debug_name)
        :
        debug_name_(std::move(debug_name))
    {}
}
