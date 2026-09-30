#shader vertex

#version 410 core

layout (location = 0) in vec2 a_Pos;
layout (location = 1) in vec4 a_Color;

out vec4 v_FragColor;
uniform mat4 u_ViewProjection;
uniform mat4 u_Model;

void main(){
    gl_Position = u_ViewProjection * u_Model * vec4(vec3(a_Pos, 0.0), 1.0);
    v_FragColor = a_Color;
}


#shader fragment

#version 410 core

in vec4 v_FragColor;
out vec4 fragColor;

void main(){
    fragColor = v_FragColor;
}