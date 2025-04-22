#pragma once
#include <Vec3.h>
#include <Matrix4.h>

enum class MoveDir { Forward, Backward, Left, Right, Up, Down };

class Camera
{
private:
	// Position & orientation (basis)
	Vec3 position_; // Camera location in world space
	Vec3 front_;    // Camera normal
	Vec3 up_, right_, worldUp_; // Up, right to form camera basis

	// Euler angles
	float yaw_, pitch_;

	// Options
	float movementSpeed;
	float mouseSensitivity;
	float zoom;    // can drive fov

	// Recompute front_/right_/up_ whenever yaw/pitch change
	void updateCameraVectors();

	// Helpers
	float deg2rad(float d) { return d * 3.14159265f / 180.0f; }

public:
	// Constructors
	Camera(const Vec3& pos, const Vec3& up, float yaw, float pitch, 
		float moveSpeed, float sens, float zoom);

	// Input reading
	void processKeyboard(MoveDir dir, float dt);
	void processMouseMovement(double xoffset, double yoffset, bool constrainPitch);
	void processMouseScroll(double yoffset);

	// Current view matrix computation
	Matrix4 getViewMatrix() const;

	// Helpers
	float getZoom() const;
	Vec3 getFront();
};

