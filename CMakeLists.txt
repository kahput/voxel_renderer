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

if(EXISTS "${CMAKE_SOURCE_DIR}/assets")
    # Set source and destination directories
    set(ASSETS_DIR "${CMAKE_SOURCE_DIR}/assets")
    set(ASSETS_DEST_DIR "${CMAKE_BINARY_DIR}/assets")

    # Get all files in the assets directory
    file(GLOB_RECURSE ASSET_FILES "${ASSETS_DIR}/*")

    # Create custom command to copy each asset file
    foreach(ASSET_FILE ${ASSET_FILES})
        # Get relative path to preserve folder structure
        file(RELATIVE_PATH REL_PATH "${ASSETS_DIR}" "${ASSET_FILE}")
        set(DEST_FILE "${ASSETS_DEST_DIR}/${REL_PATH}")

        add_custom_command(
            OUTPUT "${DEST_FILE}"
            COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${PROJECT_NAME}>/assets/${REL_PATH}"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "${ASSET_FILE}" "${DEST_FILE}"
            DEPENDS "${ASSET_FILE}"
            COMMENT "Copying asset: ${REL_PATH}"
            VERBATIM
        )

        list(APPEND ASSET_OUTPUTS "${DEST_FILE}")
    endforeach()

    # Create a custom target that depends on all copied assets
    add_custom_target(copy_assets ALL DEPENDS ${ASSET_OUTPUTS})
endif()
