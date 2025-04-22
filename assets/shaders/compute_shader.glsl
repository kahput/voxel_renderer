#version 450

layout(local_size_x = 16, local_size_y = 16) in;
layout(rgba32f, binding = 0) uniform image2D out_texture;

void main() {
    ivec2 coords = ivec2(gl_GlobalInvocationID.xy);

    // Use the total number of invocations to normalize
    // For local_size = (1,1) and dispatch = (WIDTH, HEIGHT, 1),
    // gl_NumWorkGroups.xy * gl_WorkGroupSize.xy gives the total size.
    vec2 dimensions = vec2(gl_NumWorkGroups.xy * gl_WorkGroupSize.xy);

    vec4 value;
    value.x = float(coords.x) / dimensions.x; // R channel: Normalized X (0 to ~1)
    value.y = float(coords.y) / dimensions.y; // G channel: Normalized Y (0 to ~1)
    value.z = 0.0; // B channel: 0
    value.w = 1.0; // Alpha: 1.0

    imageStore(out_texture, coords, value);
}
