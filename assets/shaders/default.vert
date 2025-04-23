#version 450

in vec2 a_position;
in vec2 a_uv;

out vec2 uv;

void main() {
    gl_Position = vec4(a_position, 0.0f, 1.0);
    uv = a_uv;
}
