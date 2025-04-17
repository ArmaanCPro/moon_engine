// basic texture shader

#type vertex
#version 460 core

layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec4 a_Color;
layout (location = 2) in vec2 a_TexCoord;

uniform mat4 u_VP = mat4(1.0);

out vec4 v_Color;
out vec2 v_TexCoord;

void main()
{
    v_Color = a_Color;
    v_TexCoord = a_TexCoord;
    gl_Position = u_VP * vec4(a_Position, 1.0);
}

#type fragment
#version 460 core
layout(location = 0) out vec4 FragColor;

in vec4 v_Color;
in vec2 v_TexCoord;

uniform sampler2D u_Texture;
uniform float u_TilingFactor = 1.0;

void main()
{
    //FragColor = texture(u_Texture, v_TexCoord * u_TilingFactor) * v_Color;
    FragColor = v_Color;
}
