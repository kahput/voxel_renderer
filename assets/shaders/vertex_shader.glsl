#version 450

in vec2 a_position;

out vec3 color;

uniform mat4 u_MVP;

void main() {
    gl_Position = u_MVP * vec4(a_position, 0.0, 1.0);
    color = vec3(a_position.xy, 0.0f);
}
