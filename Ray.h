#ifndef RAY_H
#define RAY_H

#include "Vector.h"

class Ray {

public:
	Vector origin;
	Vector direction;

	Ray(Vector orig, Vector dir) {
		origin = orig;
		direction = dir;
	}
};

#endif