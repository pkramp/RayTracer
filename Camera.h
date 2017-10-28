#ifndef CAMERA_H
#define CAMERA_H

#include "Vector.h"

class Camera {
	Vector campos, camdir, camright, camdown;

public:

	Camera();

	Camera(Vector, Vector, Vector, Vector);

	// method functions
	Vector getCameraPosition() { return campos; }
	Vector getCameraDirection() { return camdir; }
	Vector getCameraRight() { return camright; }
	Vector getCameraDown() { return camdown; }

};

Camera::Camera() {
	campos = Vector(0, 0, 0);
	camdir = Vector(0, 0, 1);
	camright = Vector(0, 0, 0);
	camdown = Vector(0, 0, 0);
}

Camera::Camera(Vector pos, Vector dir, Vector right, Vector down) {
	campos = pos;
	camdir = dir;
	camright = right;
	camdown = down;
}

#endif