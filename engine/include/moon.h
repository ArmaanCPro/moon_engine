#pragma once
// for use by client apps

#include "moon/core.h"
#include "moon/application.h"
#include "moon/log.h"
#include "moon/layer.h"
#include "moon/imgui/imgui_layer.h"

#include "moon/events/event.h"
#include "moon/events/application_event.h"
#include "moon/events/key_event.h"
#include "moon/events/mouse_event.h"

#include "moon/core/timestep.h"

#include "moon/input.h"
#include "moon/key_codes.h"
#include "moon/mouse_codes.h"

#include "moon/entry_point.h"

#include "moon/renderer/vertex_array.h"
#include "moon/renderer/buffer.h"
#include "moon/renderer/renderer.h"
#include "moon/renderer/render_command.h"
#include "moon/renderer/renderer_api.h"
#include "moon/renderer/shader.h"
#include "moon/renderer/camera.h"

//#ifndef MOON_IS_MONOLITHIC
extern "C" MOON_API ImGuiContext* moon_get_imgui_context();
//#endif
