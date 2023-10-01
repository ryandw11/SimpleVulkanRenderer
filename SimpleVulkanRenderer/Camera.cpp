#include "Camera.hpp"

Camera::Camera()
	:
	mPos{ 0, 0, 3 },
	mFront{0, 0, -1},
	mUp{0, 1, 0},
	mYaw(-90),
	mPitch(0),
	mFov(45)
{
}

glm::mat4 Camera::GetViewMatrix()
{
	return glm::lookAt(mPos, mPos + mFront, mUp);
}

void Camera::MoveForward(float speed)
{
	mPos += speed * mFront;
}

void Camera::MoveBackward(float speed)
{
	mPos -= speed * mFront;
}

void Camera::MoveLeft(float speed)
{
	mPos -= glm::normalize(glm::cross(mFront, mUp)) * speed;
}

void Camera::MoveRight(float speed)
{
	mPos += glm::normalize(glm::cross(mFront, mUp)) * speed;
}
