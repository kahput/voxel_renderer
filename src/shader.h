#pragma once

#include <stdint.h>

typedef void Shader;

Shader* opengl_shader_from_file(const char* vertex_shader_path, const char* fragment_shader_path, const char* geometry_shader_path);
Shader* opengl_shader_from_string(const char* vertex_shader_source, const char* fragment_shader_source, const char* geometry_shader_source);

Shader* opengl_shader_compute_from_file(const char* compute_shader_path);
Shader* opengl_shader_compute_from_string(const char* compute_shader_source);

void opengl_shader_destroy(Shader* shader);

void opengl_shader_activate(Shader* shader);
void opengl_shader_deactivate(Shader* shader);

void opengl_shader_seti(Shader* shader, const char* name, int32_t value);
void opengl_shader_setf(Shader* shader, const char* name, float value);
void opengl_shader_set2fv(Shader* shader, const char* name, float* value);
void opengl_shader_set3fv(Shader* shader, const char* name, float* value);
void opengl_shader_set4fv(Shader* shader, const char* name, float* value);
void opengl_shader_set2iv(Shader* shader, const char* name, int* value);
void opengl_shader_set3iv(Shader* shader, const char* name, int* value);
void opengl_shader_set4iv(Shader* shader, const char* name, int* value);
void opengl_shader_set4fm(Shader* shader, const char* name, float* value);
