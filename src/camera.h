#pragma once

typedef enum {
	PROJECTION_PERSPECTIVE,
	PROJECTION_ORTHOGRAPHIC,
	PROJECTION_FRUSTUM
} ProjectionType;

typedef struct _camera Camera;

Camera* camera_create();
void camera_destroy(Camera* camera);

void camera_set_perspective(Camera* camera, float fov, float near, float far);
void camera_set_orthogonal(Camera* camera, float size, float near, float far);

void camera_update(Camera* camera, float camera_position[3], float camera_front[3], float camera_up[3]);

float* camera_get_view(Camera* camera);
float* camera_get_projection(Camera* camera);
