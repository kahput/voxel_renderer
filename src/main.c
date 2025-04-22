#include "base.h"
#include "base/logger.h"
#include "shader.h"

#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <cglm/cglm.h>

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#define WINDOW_WDITH 1280
#define WINDOW_HEIGHT 720

typedef struct Vertex {
	vec2 position;
	vec2 uv;
} Vertex;

static const Vertex vertices[] = {
	{ { 1.f, -1.f }, { 1, 0 } },
	{ { -1.f, -1.f }, { 0, 0 } },
	{ { -1.f, 1.f }, { 0, 1 } },
	{ { 1.f, -1.f }, { 1, 0 } },
	{ { -1.f, 1.f }, { 0, 1 } },
	{ { 1.f, 1.f }, { 1, 1 } }
};

static void error_callback(int error, const char* description) {
	LOG_ERROR(description);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);
}

int main(void) {
	glfwSetErrorCallback(error_callback);

	if (!glfwInit())
		exit(EXIT_FAILURE);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	GLFWwindow* window = glfwCreateWindow(WINDOW_WDITH, WINDOW_HEIGHT, "VoxelRenderer", NULL, NULL);
	if (!window) {
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwSetKeyCallback(window, key_callback);

	glfwMakeContextCurrent(window);
	gladLoadGL(glfwGetProcAddress);

	GLuint vertex_array;
	glGenVertexArrays(1, &vertex_array);
	glBindVertexArray(vertex_array);

	GLuint vertex_buffer;
	glGenBuffers(1, &vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,
		sizeof(Vertex), (void*)offsetof(Vertex, position));

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
		sizeof(Vertex), (void*)offsetof(Vertex, uv));

	// Shader
	Shader* quad_shader = opengl_shader_from_file("assets/shaders/vertex_shader.glsl", "assets/shaders/fragment_shader.glsl", NULL);
	Shader* compute_shader = opengl_shader_compute_from_file("assets/shaders/compute_shader.glsl");

	int32_t max_group[3], max_group_size[3];
	for (uint32_t i = 0; i < 3; i++) {
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, i, &max_group[i]);
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, i, &max_group_size[i]);
	}

	LOG_INFO("GL_MAX_COMPUTE_WORK_GROUP_COUNT | [ %i, %i %i ]", max_group[0], max_group[1], max_group[2]);
	LOG_INFO("GL_MAX_COMPUTE_WORK_GROUP_SIZE | [ %i, %i %i ]", max_group_size[0], max_group_size[1], max_group_size[2]);

	// Create shader output texture

	uint32_t output_texture;
	glGenTextures(1, &output_texture);
	glBindTexture(GL_TEXTURE_2D, output_texture);
	// set the texture wrapping/filtering options (on the currently bound texture object)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, WINDOW_WDITH, WINDOW_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);

	if (output_texture) {
		glBindImageTexture(0, output_texture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
	}

	while (!glfwWindowShouldClose(window)) {
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		const float ratio = width / (float)height;

		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT);

		opengl_shader_activate(compute_shader);
		glDispatchCompute((uint32_t)WINDOW_WDITH / 16, (uint32_t)WINDOW_HEIGHT / 16, 1);

		// make sure writing to image has finished before read
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

		glBindVertexArray(vertex_array);

		opengl_shader_activate(quad_shader);
		opengl_shader_seti(quad_shader, "u_texture", 0);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, output_texture);

		glDrawArrays(GL_TRIANGLES, 0, 6);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);

	glfwTerminate();
	exit(EXIT_SUCCESS);
}
