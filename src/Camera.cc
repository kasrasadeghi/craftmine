#include "Camera.h"
#include <iostream>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <GLFW/glfw3.h>

namespace {
	float pan_speed = 0.5f;
	float roll_speed = 0.1f;
	float rotation_speed = 0.02f;
	float zoom_speed = 0.5f;
};

glm::mat4 Camera::get_view_matrix() const {
	// https://www.3dgep.com/understanding-the-view-matrix/
	using namespace glm;
	vec3 zaxis = _look;
	vec3 xaxis = right();
	vec3 yaxis = cross(xaxis, zaxis);
	mat4 orientation = {
		vec4( xaxis.x, yaxis.x, -zaxis.x, 0 ),
		vec4( xaxis.y, yaxis.y, -zaxis.y, 0 ),
		vec4( xaxis.z, yaxis.z, -zaxis.z, 0 ),
		vec4( -dot(xaxis, _eye), -dot(yaxis, _eye), dot(zaxis, _eye), 1 )
	};

	return (orientation);
}

const glm::vec3& Camera::eye() const {
	return _eye;
}

glm::vec3 Camera::right() const {
	return glm::normalize(glm::cross(_look, _up)); 
}

void Camera::zoom(float k) {
	_eye += _look * k * zoom_speed;
}

void Camera::strafe(float k) {
	_eye += right() * k * zoom_speed;
}

void Camera::vertical(float k) {
	_eye += _up * k * pan_speed;
}

void Camera::roll(float k) {
	_up = glm::rotate(_up, k * roll_speed, _look);
}

void Camera::pitch(float k) {
	_look = glm::normalize(glm::rotate(_look, k * rotation_speed, right()));
}

void Camera::yaw(float k) {
	_look = glm::normalize(glm::rotate(_look, k * rotation_speed, _up));
}

void Camera::handleKey(int key, int scancode, int action, int mods) {
	if (action == GLFW_RELEASE) {
		return;
	}

	if (key == GLFW_KEY_W) {
		zoom(1);
	} else if (key == GLFW_KEY_S) {
		zoom(-1);
	} else if (key == GLFW_KEY_A) {
		strafe(-1);
	} else if (key == GLFW_KEY_D) {
		strafe(1);
	} else if (key == GLFW_KEY_LEFT) {
		roll(-1);
	} else if (key == GLFW_KEY_RIGHT) {
		roll(1);
	} else if (key == GLFW_KEY_DOWN) {
		vertical(-1);
	} else if (key == GLFW_KEY_UP) {
		vertical(1);
	}
}