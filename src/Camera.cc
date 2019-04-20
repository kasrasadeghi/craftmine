#include "Camera.h"
#include <iostream>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/rotate_vector.hpp>

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

const glm::vec3& Camera::look() const {
	return _look;
}

const glm::vec3& Camera::up() const {
	return _up;
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
	_eye += glm::cross(right(), _look) * k * pan_speed;
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

void Camera::translate(glm::vec3 offset) {
	_eye += offset;
}

void Camera::setPos(glm::vec3 pos) {
	_eye = pos;
}