#ifndef VECTOR_H
#define VECTOR_H

#include <string>
#include <sstream>

class Vector{

public:
	double x, y, z;

	Vector() {
		this->x = 0;
		this->y = 0;
		this->z = 0;
	}

	Vector(Vector& copy) {
		this->x = copy.x;
		this->y = copy.y;
		this->z = copy.z;
	}

	Vector(double x, double y, double z) {
		this->x = x;
		this->y = y;
		this->z = z;
	}

	Vector vectAdd(Vector v) {
		return Vector(this->x + v.x, this->y + v.y, this->z + v.z);
	}

	Vector vectSub(Vector v) {
		return Vector(this->x - v.x, this->y - v.y, this->z - v.z);
	}

	Vector vectMult(double scalar) {
		return Vector(this->x*scalar, this->y*scalar, this->z*scalar);
	}
	
	double magnitude() const {
		return(sqrt(pow(this->x, 2) + pow(this->y, 2) + pow(this->z, 2)));
	}

	Vector crossProduct(Vector b) const {
		return Vector((this->y*b.z) - (this->z*b.y), (this->z*b.x) - (this->x*b.z), (this->x*b.y) - (this->y*b.x));
	}

	Vector normalize() const {
		double magnitude = this->magnitude();
		return Vector(x / magnitude, y / magnitude, z / magnitude);
	}

	Vector negative() const {
		return Vector(-x, -y, -z);
	}
};

#endif
