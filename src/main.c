#include "base.h"
#include "base/logger.h"
#include "camera.h"
#include "shader.h"
#include <cglm/mat4.h>
#include <cglm/vec3.h>

#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <cglm/cglm.h>

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720
#define VOLUME_SIZE 64

typedef struct Vertex {
	vec2 position;
	vec2 uv;
} Vertex;

typedef union _color {
	struct {
		uint32_t r, g, b, a;
	};
	uint32_t data[4];
} Color;

void print_mat4(vec4* matrix);
void generate_sphere_voxels(Color* out_array, uint32_t size);
void get_mouse_offset(GLFWwindow* window, float* x_offset, float* y_offset);

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

	GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "VoxelRenderer", NULL, NULL);
	if (!window) {
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwSetKeyCallback(window, key_callback);

	glfwMakeContextCurrent(window);
	gladLoadGL(glfwGetProcAddress);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

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

	glBindVertexArray(0);
	glDeleteBuffers(1, &vertex_buffer);

	vec3 volume_dimensions = { VOLUME_SIZE, VOLUME_SIZE, VOLUME_SIZE };
	Color colors[VOLUME_SIZE * VOLUME_SIZE * VOLUME_SIZE];
	generate_sphere_voxels(colors, 64);

	uint32_t ssbo;
	glGenBuffers(1, &ssbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(colors), colors, GL_DYNAMIC_DRAW);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // unbind

	// Shader
	Shader* quad_shader = opengl_shader_from_file("assets/shaders/default.vert", "assets/shaders/default.frag", NULL);
	Shader* compute_shader = opengl_shader_compute_from_file("assets/shaders/default.comp");

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
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);

	if (output_texture) {
		glBindImageTexture(0, output_texture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
	}

	// MVP matrices
	mat4 model;
	glm_mat4_identity(model);

	// Camera
	Camera* camera = camera_create();
	camera_set_perspective(camera, glm_rad(45.0f), 0.1f, 100.f);

	vec3 camera_position = { 0.f, 0.f, 0.f }, camera_target = { 0.0f, 0.0f, 0.0f };
	float yaw = 315.0f, pitch = 60.0f;
	const float camera_sensitivity = 4.f;

	float delta_time = 0.0f;
	float last_frame = 0.0f;

	while (!glfwWindowShouldClose(window)) {
		float current_frame = glfwGetTime();
		delta_time = current_frame - last_frame;
		last_frame = current_frame;

		int width, height;
		glfwGetFramebufferSize(window, &width, &height);

		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT);

		float x_offset = 0.0f, y_offset = 0.0f;
		get_mouse_offset(window, &x_offset, &y_offset);

		yaw += x_offset * delta_time * camera_sensitivity;
		yaw = yaw > 360.f ? 0.f : yaw < 0.0f ? 360.f
											 : yaw;
		pitch += y_offset * delta_time * camera_sensitivity;
		pitch = (5.0f > (105.0f < pitch ? 105.0f : pitch) ? 5.0f : (105.0f < pitch ? 105.0f : pitch));

		camera_position[0] = (VOLUME_SIZE * 5.f) * cos(glm_rad(yaw)) * sin(glm_rad(pitch));
		camera_position[1] = (VOLUME_SIZE * 5.f) * cos(glm_rad(pitch));
		camera_position[2] = (VOLUME_SIZE * 5.f) * sin(glm_rad(yaw)) * sin(glm_rad(pitch));

		glm_vec3_scale(camera_target, 0, camera_target);

		glm_vec3_sub(camera_target, camera_position, camera_target);
		camera_update(camera, camera_position, camera_target, (vec3){ 0.0f, 1.0f, 0.0f });

		mat4 inverse_view, inverse_projection;

		glm_mat4_identity(inverse_view);
		glm_mat4_identity(inverse_projection);

		glm_mat4_inv((vec4*)camera_get_view(camera), inverse_view);
		glm_mat4_inv((vec4*)camera_get_projection(camera), inverse_projection);

		opengl_shader_activate(compute_shader);

		// print_mat4(inverse_view);

		// Upload camera data to compute shader
		opengl_shader_set4fm(compute_shader, "uClipToCamera", (float*)inverse_projection);
		opengl_shader_set4fm(compute_shader, "uCameraToWorld", (float*)inverse_view);
		opengl_shader_setf(compute_shader, "uTime", (float)glfwGetTime());

		// Upload voxel data to compute shader
		opengl_shader_set3fv(compute_shader, "uVolumeDimension", volume_dimensions);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);

		glDispatchCompute((uint32_t)WINDOW_WIDTH / 16, (uint32_t)WINDOW_HEIGHT / 16, 1);

		// make sure writing to image has finished before read
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

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

void generate_sphere_voxels(Color* out_array, uint32_t size) {
	float radius = size / 2.f;

	for (int z = 0; z < size; ++z) {
		for (int y = 0; y < size; ++y) {
			for (int x = 0; x < size; ++x) {
				uint32_t index = x + y * size + (z * size * size);
				vec3 delta = {
					(x + 0.5f) - radius,
					(y + 0.5f) - radius,
					(z + 0.5f) - radius,
				};

				if ((glm_dot(delta, delta) - 0.1f) <= radius * radius) {
					// Inside sphere
					out_array[index] = (Color){
						.r = 255,
						.g = 128,
						.b = 64,
						.a = 255
					};

				} else {
					// Outside sphere
					out_array[index] = (Color){ 0 };
				}
			}
		}
	}
}

void print_mat4(vec4* matrix) {
	LOG_INFO("mat4 value: \n[%.1f, %.1f, %.1f, %.1f]\n[%.1f, %.1f, %.1f, %.1f]\n[%.1f, %.1f, %.1f, %.1f]\n[%.1f, %.1f, %.1f, %.1f]\n",
		matrix[0][0],
		matrix[0][1],
		matrix[0][2],
		matrix[0][3],
		matrix[1][0],
		matrix[1][1],
		matrix[1][2],
		matrix[1][3],
		matrix[2][0],
		matrix[2][1],
		matrix[2][2],
		matrix[2][3],
		matrix[3][0],
		matrix[3][1],
		matrix[3][2],
		matrix[3][3]);
}
void get_mouse_offset(GLFWwindow* window, float* x_offset, float* y_offset) {
	static uint32_t current_frame = 0;
	static double last_position_x = 0.0f, last_position_y = 0.0f;

	double current_position_x, current_position_y;
	glfwGetCursorPos(window, &current_position_x, &current_position_y);

	if (current_frame == 0) {
		last_position_x = WINDOW_WIDTH / 2.f;
		last_position_y = WINDOW_HEIGHT / 2.f;
		current_frame++;
		return;
	}

	*x_offset = current_position_x - last_position_x;
	*y_offset = last_position_y - current_position_y;

	last_position_x = current_position_x;
	last_position_y = current_position_y;
}
