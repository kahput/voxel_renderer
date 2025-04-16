add_subdirectory("glfw")
add_subdirectory("glad/cmake")
add_subdirectory("cglm" EXCLUDE_FROM_ALL)

# Specify glad settings
glad_add_library(glad_gl_core_45 REPRODUCIBLE API gl:core=4.5)
