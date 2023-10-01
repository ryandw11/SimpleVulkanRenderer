#pragma once

#ifndef CAMERA_H
#define CAMERA_H

#include "VulkanIncludes.hpp"

class Camera {
public:
	Camera();

	glm::mat4 GetViewMatrix();
	void MoveForward(float speed);
	void MoveBackward(float speed);
	void MoveLeft(float speed);
	void MoveRight(float speed);

private:
	glm::vec3 mPos;
	glm::vec3 mFront;
	glm::vec3 mUp;
	float mYaw;
	float mPitch;
	float mFov;
};

#endif