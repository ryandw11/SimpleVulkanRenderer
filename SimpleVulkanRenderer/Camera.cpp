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

void Camera::mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (mFirstMouse)
    {
        mLastX = xpos;
        mLastY = ypos;
        mFirstMouse = false;
    }

    float xoffset = xpos - mLastX;
    float yoffset = mLastY - ypos; // reversed since y-coordinates go from bottom to top
    mLastX = xpos;
    mLastY = ypos;

    float sensitivity = 0.1f; // change this value to your liking
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    mYaw += xoffset;
    mPitch += yoffset;

    // make sure that when pitch is out of bounds, screen doesn't get flipped
    if (mPitch > 89.0f)
        mPitch = 89.0f;
    if (mPitch < -89.0f)
        mPitch = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(mYaw)) * cos(glm::radians(mPitch));
    front.y = sin(glm::radians(mPitch));
    front.z = sin(glm::radians(mYaw)) * cos(glm::radians(mPitch));
    mFront = glm::normalize(front);
}