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
	Vec3 horiz = right_.cross(worldUp_) * -1 ;
	horiz.normalize();

	if (dir == MoveDir::Forward)  position_ = position_ + horiz * vel;
	if (dir == MoveDir::Backward) position_ = position_ - horiz * vel;
	if (dir == MoveDir::Left)     position_ = position_ - right_ * vel;
	if (dir == MoveDir::Right)    position_ = position_ + right_ * vel;
	if (dir == MoveDir::Up)       position_ = position_ - worldUp_ * vel;
	if (dir == MoveDir::Down)     position_ = position_ + worldUp_ * vel;
}
void Camera::processMouseMovement(double xoffset, double yoffset, bool constrainPitch)
{
	xoffset *= mouseSensitivity;
	yoffset *= mouseSensitivity;

	// Adjust yaw and pitch by offset
	yaw_ += float(xoffset);
	pitch_ -= float(yoffset);

	// Clamp the pitch (fix looking up and down all the way)
	if (constrainPitch)
	{
		if (pitch_ > 89.0f) pitch_ = 89.0f;
		if (pitch_ < -89.0f) pitch_ = -89.0f;
	}

	if (yaw_ > 360.0f) yaw_ -= 360.0f;
	if (yaw_ < -360.0f) yaw_ += 360.0f;

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
// We form the camera basis, {facing, right, trueUp}, and then rotate points in 
// world space to camera space, aligning its normal with the -Z axis. Then, we want
// all points to be centered on the camera, so that means simply translate (subtract)
// them to eye now that its all rotated nicely. This is just a change of basis 
// on world space points -> camera space, where the camera looks down its own -z axis. 
// Then, we simply translate all the points such that they exist relative to the camera,
// i.e. the new origin.
//
// The camera will then 'see' the points from its perspective and we can pass those along,
// eventually to the framebuffer.
Matrix4 Camera::getViewMatrix() const
{	
	// 2) start with identity
	Matrix4 V = Matrix4::identity();

	// 3) fill the rotation (upper-left 3x3) 
	//    dot product of p with r, u, -f (projection)
	//    [ r.x  u.x  -f.x ]
	//    [ r.y  u.y  -f.y ]
	//    [ r.z  u.z  -f.z ]
	// rows = camera basis (row-major)
	V(0, 0) = right_.x_;    V(0, 1) = right_.y_;    V(0, 2) = right_.z_;
	V(1, 0) = up_.x_;       V(1, 1) = up_.y_;       V(1, 2) = up_.z_;   
	V(2, 0) = -front_.x_;   V(2, 1) = -front_.y_;   V(2, 2) = -front_.z_;  
	// bottom row stays [0 0 0 1]
	
	// 4) fill the translation (last column)
	V(0, 3) = -right_.dot(position_);
	V(1, 3) = -up_.dot(position_);
	V(2, 3) = front_.dot(position_);

	return V;
}

// Build projection matrix
// Transforms each world space coordinate to the near clipping plane
Matrix4 Camera::getProjMatrix(float fovY, float aspect,
	float near, float far) const
{
	// Start with a zero matrix
	Matrix4 P;

	// Compute focal scale from vertical FOV
	float f = 1.0f / std::tan(fovY * 0.5f);

	// X and Y scale
	P(0, 0) = f / aspect;
	P(1, 1) = f;

	// Z remap: [near,far] -> [-1,1]
	P(2, 2) = -(far + near) / (far - near);
	P(2, 3) = -2.0f * far * near / (far - near);

	// W component to perform perspective divide
	P(3, 2) = -1.0f;

	return P;
}

// Helpers
float Camera::getZoom() const
{
	return zoom;
}
Vec3 Camera::getFront()
{
	return front_;
}