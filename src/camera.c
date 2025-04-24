#include "camera.h"
#include "base.h"

#include <cglm/cam.h>
#include <cglm/cglm.h>
#include <cglm/mat4.h>
#include <stdio.h>
#include <stdlib.h>

struct _camera {
	float frustum, near, far; // Frustum = fov in perspective, Frustum = box_size in orthographic
	uint32_t projection_type, projection_dirty; // Camera projection: CAMERA_PERSPECTIVE or CAMERA_ORTHOGRAPHIC
	mat4 view_matrix, projection_matrix;
};

Camera* camera_create() {
	Camera* camera = malloc(sizeof(Camera));

	*camera = (Camera){ .frustum = 0.f, .near = 0.f, .far = 0.f, .projection_type = PROJECTION_FRUSTUM, .projection_dirty = false };
	glm_mat4_identity(camera->view_matrix);
	glm_mat4_identity(camera->projection_matrix);

	return camera;
}
void camera_destroy(Camera* camera) {
	if (camera)
		free(camera);
}

void camera_set_perspective(Camera* camera, float fov, float near, float far) {
	camera->projection_type = PROJECTION_PERSPECTIVE;
	camera->near = near, camera->far = far, camera->frustum = fov;
	camera->projection_dirty = true;
}

void camera_set_orthogonal(Camera* camera, float size, float near, float far) {
	camera->projection_type = PROJECTION_ORTHOGRAPHIC;
	camera->near = near, camera->far = far, camera->frustum = size;
	camera->projection_dirty = true;
}

void camera_update(Camera* camera, float camera_position[3], float camera_front[3], float camera_up[3]) {
	if (camera->projection_dirty)
		switch (camera->projection_type) {
			case PROJECTION_PERSPECTIVE: {
				glm_perspective(camera->frustum, 16.f / 9.f, camera->near, camera->far, camera->projection_matrix);
				camera->projection_dirty = false;
			} break;
			case PROJECTION_ORTHOGRAPHIC: {
				glm_ortho(-(camera->frustum * 0.5f), (camera->frustum * 0.5f), -(camera->frustum * 0.5f), (camera->frustum * 0.5f), camera->near, camera->far, camera->projection_matrix);
				camera->projection_dirty = false;
			} break;
			case PROJECTION_FRUSTUM: {
				LOG_WARN("Camera projection not initialized!");
			} break;
		}
	glm_look(camera_position, camera_front, camera_up, camera->view_matrix);
}

float* camera_get_view(Camera* camera) {
	return (float*)camera->view_matrix;
}
float* camera_get_projection(Camera* camera) {
	return (float*)camera->projection_matrix;
}
