#type vertex
#version 460 core
layout (location = 0) in vec3 a_Position;

uniform mat4 u_VP = mat4(1.0);
uniform mat4 u_Model = mat4(1.0);

out vec3 v_Pos;

void main()
{
    v_Pos = a_Position;
    gl_Position = u_VP * u_Model * vec4(a_Position, 1.0);
}


#type fragment
#version 460 core
layout(location = 0) out vec4 FragColor;

uniform vec4 u_Color;

in vec3 v_Pos;

void main()
{
    //vec3 circle = vec3(v_Pos.x / 2.0, v_Pos.y / 2.0, 0.1);
    //float d = length(v_Pos.xy - circle.xy) - circle.z;
    //d = smoothstep(0.0, 0.001, d);

    float d = 0.0;
    FragColor = u_Color * (1 - d);
}
