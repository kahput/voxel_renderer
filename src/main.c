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

typedef struct Vertex {
	vec3 pos;
} Vertex;

static const Vertex vertices[] = {
	{ { 1.f, -1.f } },
	{ { -1.f, -1.f } },
	{ { -1.f, 1.f } },
	{ { 1.f, -1.f } },
	{ { -1.f, 1.f } },
	{ { 1.f, 1.f } }
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

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	GLFWwindow* window = glfwCreateWindow(1280, 720, "OpenGL Triangle", NULL, NULL);
	if (!window) {
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwSetKeyCallback(window, key_callback);

	glfwMakeContextCurrent(window);
	gladLoadGL(glfwGetProcAddress);

	// NOTE: OpenGL error checks have been omitted for brevity

	GLuint vertex_buffer;
	glGenBuffers(1, &vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

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

	GLuint vertex_array;
	glGenVertexArrays(1, &vertex_array);
	glBindVertexArray(vertex_array);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,
		sizeof(Vertex), (void*)offsetof(Vertex, pos));

	while (!glfwWindowShouldClose(window)) {
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		const float ratio = width / (float)height;

		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT);

		mat4 m, p, mvp;
		glm_mat4_identity(m);
		glm_ortho(-1.f, 1.f, -1.f, 1.f, 1.f, -1.f, p);
		glm_mat4_mul(p, m, mvp);

		opengl_shader_activate(quad_shader);
		opengl_shader_set4fm(quad_shader, "u_MVP", (float*)mvp);
		glBindVertexArray(vertex_array);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);

	glfwTerminate();
	exit(EXIT_SUCCESS);
}
