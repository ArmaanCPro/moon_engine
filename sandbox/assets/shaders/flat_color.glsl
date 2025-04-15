#type vertex
#version 460 core
layout (location = 0) in vec3 a_Pos;

uniform mat4 u_VP = mat4(1.0);
uniform mat4 u_Model = mat4(1.0);

void main()
{
    gl_Position = u_VP * u_Model * vec4(a_Pos, 1.0);
}


#type fragment
#version 460 core
layout(location = 0) out vec4 FragColor;

uniform vec4 u_Color;

void main()
{
    FragColor = u_Color;
}
