#include "Camera.h"

// Constructors
Camera::Camera(const Vec3& pos, const Vec3& up, float yaw, float pitch, 
	float moveSpeed, float sense, float zoom) : position_(pos), worldUp_(up),
		yaw_(yaw), pitch_(pitch), movementSpeed(moveSpeed), 
			mouseSensitivity(sense), zoom(zoom) 
{
	updateCameraVectors();
}

// Input reading
void Camera::processKeyboard(MoveDir dir, float dt)
{
	float vel = movementSpeed * dt;
	Vec3 horiz(front_[0], 0, front_[2]);
	horiz.normalize();

	if (dir == MoveDir::Forward)  position_ = position_ - front_ * vel;
	if (dir == MoveDir::Backward) position_ = position_ + front_ * vel;
	if (dir == MoveDir::Left)     position_ = position_ + right_ * vel;
	if (dir == MoveDir::Right)    position_ = position_ - right_ * vel;
	if (dir == MoveDir::Up)       position_ = position_ + worldUp_ * vel;
	if (dir == MoveDir::Down)     position_ = position_ - worldUp_ * vel;
}
void Camera::processMouseMovement(double xoffset, double yoffset, bool constrainPitch)
{
	xoffset *= mouseSensitivity;
	yoffset *= mouseSensitivity;

	// Just add our mouse offset to the yaw and pitch, they dont need to
	// scale as angles
	yaw_ += float(xoffset);
	pitch_ -= float(yoffset);

	// Clamp the pitch (fix looking up and down all the way)
	if (constrainPitch)
	{
		if (pitch_ > 89.0f) pitch_ = 89.0f;
		if (pitch_ < -89.0f) pitch_ = -89.0f;
	}

	updateCameraVectors();
}
void Camera::processMouseScroll(double yoffset)
{
	zoom -= float(yoffset);
	if (zoom < 1.0f) zoom = 1.0f;
	if (zoom > 45.0f) zoom = 45.0f;
}

// Camera basis update on mouse movement
void Camera::updateCameraVectors() 
{
	// 1) spherical -> Cartesian for front_
	Vec3 f;
	f.x_ = cos(deg2rad(yaw_)) * cos(deg2rad(pitch_));
	f.y_ = sin(deg2rad(pitch_));
	f.z_ = sin(deg2rad(yaw_)) * cos(deg2rad(pitch_));
	front_ = f.normalize();

	// 2) build orthonormal basis
	right_ = front_.cross(worldUp_).normalize(); // f x Wup = r
	up_ = right_.cross(front_).normalize(); // r x f = Camup
}

// Build view matrix
Matrix4 Camera::getViewMatrix() const
{
	return Matrix4::lookAt(position_, position_ + front_, up_);
}

// Helpers
float Camera::getZoom()
{
	return zoom;
}