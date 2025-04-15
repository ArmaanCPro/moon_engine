// basic texture shader

#type vertex
#version 460 core
layout (location = 0) in vec3 a_Pos;
layout (location = 1) in vec2 a_TexCoord;

uniform mat4 u_VP = mat4(1.0);
uniform mat4 u_Model = mat4(1.0);

out vec2 v_TexCoord;

void main()
{
    v_TexCoord = a_TexCoord;
    gl_Position = u_VP * u_Model * vec4(a_Pos, 1.0);
}

#type fragment
#version 460 core
layout(location = 0) out vec4 FragColor;

in vec2 v_TexCoord;

uniform sampler2D u_Texture;
uniform vec4 u_Color;

void main()
{
    FragColor = texture(u_Texture, v_TexCoord * 10.0) * u_Color;
}
