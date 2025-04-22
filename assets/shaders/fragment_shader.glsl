#version 450

in vec2 uv;

out vec4 fragment;

uniform sampler2D u_texture;

void main() {
    fragment = texture(u_texture, uv);
}
