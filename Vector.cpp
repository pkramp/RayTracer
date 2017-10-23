#include "Vector.h"

Vector::Vector() {}

Vector::Vector(float x, float y, float z) {
	this->x = x;
	this->y = y;
	this->z = z;
}

Vector Vector::operator+ (const Vector other) const {
	return Vector(this->x + other.x, this->y + other.y, this->z + other.z);
}

Vector Vector::operator- (const Vector other) const {
	return Vector(this->x - other.x, this->y - other.y, this->z - other.z);
}

float Vector::operator*(const Vector b) const {
	return float((this->x*b.x) + (this->y*b.y) + (this->z*b.z));
}

Vector Vector::operator*(const float factor) const {
	return Vector(this->x * factor, this->y * factor, this->z * factor);
}

bool Vector::operator==(const Vector& other) const {
	//return this->x == other.x && this->y == other.y &&  this->z == other.z;
	if (roundf(this->x) == roundf(other.x))
		if (roundf(this->y) == roundf(other.y))
			if (roundf(this->z) == roundf(other.z))
				return true;
	return false;
}

bool Vector::operator!=(const Vector& other) const {
	//return roundf(this->x) == roundf(other.x) && roundf(this->y) == roundf(other.y) && roundf(this->z) == roundf(other.z);
	float accuracy = 0.000001f;
	return std::abs(this->x - other.x) < accuracy && 
		std::abs(this->y - other.y) < accuracy && 
		std::abs(this->z - other.z) < accuracy;
	/*if (roundf(this->x) == roundf(other.x))
		if (roundf(this->y) == roundf(other.y))
			if (roundf(this->z) == roundf(other.z))
				return false;
	return true;*/
}

float Vector::abs()const {
	return(sqrt(x*x + y*y + z*z));
}

Vector Vector::cross(Vector b) {
	return Vector((this->y*b.z) - (this->z*b.y), (this->z*b.x) - (this->x*b.z), (this->x*b.y) - (this->y*b.x));
}

Vector Vector::normalize() const {
	float factor = 1 / this->abs();
	return Vector(factor*x, factor*y, factor*z);
}

std::string Vector::toString() {
	std::stringstream out;

	out << this->x << " " << this->y << " " << this->z << std::endl;

	return out.str();
}


