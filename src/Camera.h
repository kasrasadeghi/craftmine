#pragma once 
#include <glm/glm.hpp>

class Camera {
public:
	static constexpr float pan_speed = 0.2f;
	static constexpr float roll_speed = 0.1f;
	static constexpr float rotation_speed = 0.02f;
	static constexpr float zoom_speed = 0.2f;

	glm::mat4 get_view_matrix() const;
	const glm::vec3& eye() const;
	const glm::vec3& look() const;
	const glm::vec3& up() const;

	void translate(glm::vec3 offset);
	void setPos(glm::vec3 pos);

	void zoom(float k);
	void strafe(float k);
	void vertical(float k);
	void roll(float k);
	void pitch(float k);
	void yaw(float k);

private:
	glm::vec3 right() const;
	glm::vec3 _eye = glm::vec3(20, 100, 20);
	glm::vec3 _look = glm::normalize(glm::vec3(1, -1.5, 1));
	glm::vec3 _up = glm::vec3(0.0f, 1.0, 0.0f);
};
