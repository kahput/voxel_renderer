#version 450

layout(local_size_x = 32, local_size_y = 32) in;
layout(rgba32f, binding = 0) uniform image2D out_texture;

void main() {
    vec4 value = vec4(0.0, 0.0, 0.0, 1.0);
    ivec2 texture_coordinate = ivec2(gl_GlobalInvocationID.xy);

    value.x = float(texture_coordinate.x) / (gl_NumWorkGroups.x * gl_WorkGroupSize.x);
    value.y = float(texture_coordinate.y) / (gl_NumWorkGroups.y * gl_WorkGroupSize.y);

    imageStore(out_texture, texture_coordinate, value);
}
