#pragma once
#include <math.h>
#include <algorithm>
#include "Vector.h"
#include "Ray.h"

using namespace std;
class Box {

public:
	Box(Vector min, Vector max) {
		this->min = min;
		this->max = max;
		float diffX = abs(min.x - max.x);
		float diffY = abs(min.y - max.y);
		float diffZ = abs(min.z - max.z);
		this->center = Vector(max.x - 0.5 * diffX, max.y - 0.5 * diffY, max.z - 0.5 * diffZ);
	}

	Vector min;
	Vector max;
	Vector center;

	bool intersect(const Ray &r)
	{
		float tmin = (min.x - r.origin.x) / r.direction.x;
		float tmax = (max.x - r.origin.x) / r.direction.x;

		if (tmin > tmax) swap(tmin, tmax);

		float tymin = (min.y - r.origin.y) / r.direction.y;
		float tymax = (max.y - r.origin.y) / r.direction.y;

		if (tymin > tymax) swap(tymin, tymax);

		if ((tmin > tymax) || (tymin > tmax))
			return false;

		if (tymin > tmin)
			tmin = tymin;

		if (tymax < tmax)
			tmax = tymax;

		float tzmin = (min.z - r.origin.z) / r.direction.z;
		float tzmax = (max.z - r.origin.z) / r.direction.z;

		if (tzmin > tzmax) swap(tzmin, tzmax);

		if ((tmin > tzmax) || (tzmin > tmax))
			return false;

		if (tzmin > tmin)
			tmin = tzmin;

		if (tzmax < tmax)
			tmax = tzmax;

		return true;
	}

};
