#pragma once
// for use by client apps

#include "core/core.h"
#include "core/application.h"
#include "core/log.h"
#include "core/layer.h"
#include "core/imgui/imgui_layer.h"

#include "core/events/event.h"
#include "core/events/application_event.h"
#include "core/events/key_event.h"
#include "core/events/mouse_event.h"

#include "core/core/timestep.h"

#include "core/input.h"
#include "core/key_codes.h"
#include "core/mouse_codes.h"

#include "core/entry_point.h"

#include "core/renderer/vertex_array.h"
#include "core/renderer/buffer.h"
#include "core/renderer/renderer.h"
#include "core/renderer/render_command.h"
#include "core/renderer/renderer_api.h"
#include "core/renderer/shader.h"
#include "core/renderer/camera.h"

//#ifndef MOON_IS_MONOLITHIC
extern "C" MOON_API ImGuiContext* moon_get_imgui_context();
//#endif
