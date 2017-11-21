#ifndef _OBJECT_H
#define _OBJECT_H

#include "Ray.h"
#include "Vector.h"
#include "Colour.h"


class Object {
public:
	Colour colour;

	Object() {};
	Object(Colour col) {
		this->colour = col;
	};

	virtual Vector getNormalAt(Vector intersection_position, Vector intersection_to_light_direction) {
		return Vector(0, 0, 0);
	}

	virtual double intersect(Ray ray, Vector& intersectionPoint) {
		return 0;
	}

};


#endif