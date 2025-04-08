#pragma once

// stl
#include <iostream>
#include <memory>
#include <utility>
#include <algorithm>
#include <functional>
#include <sstream>
#include <fstream>
#include <cstdint>

#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <array>

// graphics (glad must be included before glfw)
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// our stuff
#include "core/log.h"

#ifdef _WIN32
    #include <windows.h>
#endif


