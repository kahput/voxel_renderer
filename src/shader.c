#include "shader.h"
#include "base.h"

#include <glad/gl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct _gl_shader {
	uint32_t id; // Shader program id
	int32_t* locations; // TODO: Use this
} OpenGLShader;

Shader* opengl_shader_compute_from_file(const char* compute_shader_path) {
	FILE* file_ptr = fopen(compute_shader_path, "r");
	if (!file_ptr) {
		LOG_ERROR("VERTEX_SHADER_FILE [ %s ] NOT_FOUND", compute_shader_path);
		return NULL;
	}

	fseek(file_ptr, 0, SEEK_END);
	uint32_t length = ftell(file_ptr);
	char compute_shader_source[length + 1];
	fseek(file_ptr, 0, SEEK_SET);
	fread(compute_shader_source, 1, length, file_ptr);
	fclose(file_ptr);
	compute_shader_source[length] = '\0';

	return opengl_shader_compute_from_string(compute_shader_source);
}

Shader* opengl_shader_compute_from_string(const char* compute_shader_source) {
	OpenGLShader* shader = malloc(sizeof(OpenGLShader));
	uint32_t compute_shader = glCreateShader(GL_COMPUTE_SHADER);
	glShaderSource(compute_shader, 1, &compute_shader_source, NULL);
	glCompileShader(compute_shader);

	int32_t success;
	char info_buffer[512];
	glGetShaderiv(compute_shader, GL_COMPILE_STATUS, &success);

	if (!success) {
		glGetShaderInfoLog(compute_shader, 512, NULL, info_buffer);
		LOG_ERROR("SHADER_COMPUTE_COMPILATION_FAILED | %s", info_buffer);
		return NULL;
	}

	uint32_t program = glCreateProgram();
	glAttachShader(program, compute_shader);
	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(program, 512, NULL, info_buffer);
		LOG_ERROR("SHADER_LINKING_FAILED | %s", info_buffer);
		return NULL;
	}

	shader->id = program;
	glDeleteShader(compute_shader);

	return shader;
}

Shader* opengl_shader_from_file(const char* vertex_shader_path, const char* fragment_shader_path, const char* geometry_shader_path) {
	// Vertex shader
	FILE* file_ptr = fopen(vertex_shader_path, "r");
	if (!file_ptr) {
		LOG_ERROR("VERTEX_SHADER_FILE [ %s ] NOT_FOUND", vertex_shader_path);
		return NULL;
	}

	fseek(file_ptr, 0, SEEK_END);
	uint32_t length = ftell(file_ptr);
	char vertex_shader_source[length + 1];
	fseek(file_ptr, 0, SEEK_SET);
	fread(vertex_shader_source, 1, length, file_ptr);
	fclose(file_ptr);
	vertex_shader_source[length] = '\0';

	// Fragment shader
	file_ptr = fopen(fragment_shader_path, "r");
	if (!file_ptr) {
		LOG_ERROR("FRAGMENT_SHADER_FILE [ %s ] NOT_FOUND", fragment_shader_path);
		return NULL;
	}

	fseek(file_ptr, 0, SEEK_END);
	length = ftell(file_ptr);
	char fragment_shader_source[length + 1];
	fseek(file_ptr, 0, SEEK_SET);
	fread(fragment_shader_source, 1, length, file_ptr);
	fclose(file_ptr);
	fragment_shader_source[length] = '\0';

	return opengl_shader_from_string(vertex_shader_source, fragment_shader_source, NULL);
}

Shader* opengl_shader_from_string(const char* vertex_shader_source, const char* fragment_shader_source, const char* geometry_shader_source) {
	OpenGLShader* shader = malloc(sizeof(OpenGLShader));
	uint32_t vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex_shader, 1, &vertex_shader_source, NULL);
	glCompileShader(vertex_shader);

	int32_t success;
	char info_buffer[512];
	glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);

	if (!success) {
		glGetShaderInfoLog(vertex_shader, 512, NULL, info_buffer);
		LOG_ERROR("SHADER_VERTEX_COMPILATION_FAILED | %s", info_buffer);
		return NULL;
	}

	uint32_t fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader, 1, &fragment_shader_source, NULL);
	glCompileShader(fragment_shader);

	glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);

	if (!success) {
		glGetShaderInfoLog(fragment_shader, 512, NULL, info_buffer);
		LOG_ERROR("SHADER_FRAGMENT_COMPILATION_FAILED | %s", info_buffer);
		return NULL;
	}

	uint32_t program = glCreateProgram();
	glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);
	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(program, 512, NULL, info_buffer);
		LOG_ERROR("SHADER_LINKING_FAILED | %s", info_buffer);
		return NULL;
	}

	shader->id = program;
	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

	return shader;
}
void opengl_shader_destroy(Shader* shader) {
	OpenGLShader* gl_shader = (OpenGLShader*)shader;
	glDeleteProgram(gl_shader->id);
	free(shader);
}

void opengl_shader_activate(Shader* shader) {
	OpenGLShader* gl_shader = (OpenGLShader*)shader;
	glUseProgram(gl_shader->id);
}
void opengl_shader_deactivate(Shader* shader) {
	glUseProgram(0);
}

void opengl_shader_seti(Shader* shader, const char* name, int32_t value) {
	OpenGLShader* gl_shader = (OpenGLShader*)shader;
	glUniform1i(glGetUniformLocation(gl_shader->id, name), value);
}
void opengl_shader_setf(Shader* shader, const char* name, float value) {
	OpenGLShader* gl_shader = (OpenGLShader*)shader;
	glUniform1f(glGetUniformLocation(gl_shader->id, name), value);
}
void opengl_shader_set2fv(Shader* shader, const char* name, float* value) {
	OpenGLShader* gl_shader = (OpenGLShader*)shader;
	glUniform2fv(glGetUniformLocation(gl_shader->id, name), 1, value);
}
void opengl_shader_set3fv(Shader* shader, const char* name, float* value) {
	OpenGLShader* gl_shader = (OpenGLShader*)shader;
	glUniform3fv(glGetUniformLocation(gl_shader->id, name), 1, value);
}
void opengl_shader_set4fv(Shader* shader, const char* name, float* value) {
	OpenGLShader* gl_shader = (OpenGLShader*)shader;
	glUniform4fv(glGetUniformLocation(gl_shader->id, name), 1, value);
}

void opengl_shader_set2iv(Shader* shader, const char* name, int* value) {
	OpenGLShader* gl_shader = (OpenGLShader*)shader;
	glUniform2iv(glGetUniformLocation(gl_shader->id, name), 1, value);
}

void opengl_shader_set3iv(Shader* shader, const char* name, int* value) {
	OpenGLShader* gl_shader = (OpenGLShader*)shader;
	glUniform3iv(glGetUniformLocation(gl_shader->id, name), 1, value);
}

void opengl_shader_set4iv(Shader* shader, const char* name, int* value) {
	OpenGLShader* gl_shader = (OpenGLShader*)shader;
	glUniform4iv(glGetUniformLocation(gl_shader->id, name), 1, value);
}

void opengl_shader_set4fm(Shader* shader, const char* name, float* value) {
	OpenGLShader* gl_shader = (OpenGLShader*)shader;
	glUniformMatrix4fv(glGetUniformLocation(gl_shader->id, name), 1, GL_FALSE, value);
}
