cmake_minimum_required(VERSION 3.5)
project(voxel_renderer)
set(CMAKE_C_STANDARD 99)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(ext)

file(GLOB_RECURSE SOURCES "src/*.c" "src/*.h" "ext/stb_image/*.c" "ext/stb_image/*.h")
add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries( ${PROJECT_NAME}
    PUBLIC
        glad_gl_core_45
        glfw
        cglm_headers
    )
