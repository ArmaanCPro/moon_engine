#pragma once
// for use by client apps

#include "moon/core/core.h"
#include "moon/core/application.h"
#include "moon/core/log.h"
#include "moon/core/layer.h"
#include "moon/imgui/imgui_layer.h"

#include "moon/debug/instrumentor.h"

#include "moon/events/event.h"
#include "moon/events/application_event.h"
#include "moon/events/key_event.h"
#include "moon/events/mouse_event.h"

#include "moon/core/timestep.h"

#include "moon/core/input.h"
#include "moon/core/key_codes.h"
#include "moon/core/mouse_codes.h"

#include "moon/renderer/renderer.h"
#include "moon/renderer/renderer2d.h"
#include "moon/renderer/render_command.h"
#include "moon/renderer/renderer_api.h"
#include "moon/renderer/vertex_array.h"
#include "moon/renderer/buffer.h"
#include "moon/renderer/shader.h"
#include "moon/renderer/camera.h"
#include "moon/renderer/texture.h"
#include "moon/renderer/subtexture2d.h"
#include "moon/renderer/orthographic_camera_controller.h"
#include "moon/renderer/handle.h"

#include "moon/scene/scene.h"
#include "moon/scene/entity.h"
#include "moon/scene/components.h"

//#ifndef MOON_IS_MONOLITHIC
struct ImGuiContext;
extern "C" MOON_API ImGuiContext* moon_get_imgui_context();
//#endif
