#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>

class Camera {
public:
	glm::mat4 get_view_matrix() const;
	// FIXME: add functions to manipulate camera objects.
	void zoom(float k);
	void strafe(float k);
	void vertical(float k);
	void roll(float k);
	void pitch(float k);
	void yaw(float k);
	const glm::vec3& eye() const;

	void handleKey(int key, int scancode, int action, int mods);
private:
	glm::vec3 right() const;
	glm::vec3 up() const;
	glm::vec3 _eye = glm::vec3(10, 55, 0);
	glm::vec3 _look = glm::normalize(glm::vec3(-1, 0, 0));
	glm::vec3 _up = glm::vec3(0.0f, 1.0, 0.0f);
};

#endif
