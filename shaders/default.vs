#version 330

layout (location = 0) in vec3 position;


uniform mat4 model;
uniform mat4 view;

void main () {

    vec4 tmp4 = view * model * vec4 (position, 1.0);
    gl_Position = tmp4;
}