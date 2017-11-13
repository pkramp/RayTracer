#pragma once

#include "Vector.h"
#include "Ray.h"
#include "Object.h"
#include "Colour.h"

class Plane : public Object {
public:
	Colour colour;
	Vector point;
	Vector normal;

	Plane(Vector point, Vector normal, Colour col) : Object(col){
		this->point = point;
		this->normal = normal.normalize();
		this->colour = col;
}
	double intersect(Ray ray) {
		// assuming vectors are all normalized
		float denom = normal.dotProduct( ray.direction.normalize());
		if (denom > 1e-6) {
			Vector p0l0 = point.vectSub(ray.origin);
			double t = p0l0.dotProduct(normal) / denom;
			return t;
		}

		return -1;
	}

	Vector getNormalAt(Vector point) {
		return normal;
	}

};