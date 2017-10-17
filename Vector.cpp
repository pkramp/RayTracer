#include "Vector.h"

Vector::Vector(){}

Vector::Vector(double x, double y, double z){
	this->x = x;
	this->y = y;
	this->z = z;
}

Vector Vector::operator+ (Vector other){
	return Vector(this->x + other.x, this->y + other.y, this->z + other.z);
}

Vector Vector::operator- (Vector other){
	return Vector(this->x - other.x, this->y - other.y, this->z - other.z);
}

double Vector::operator*(Vector b){
	return double((this->x*b.x) + (this->y*b.y) + (this->z*b.z));
}

Vector Vector::operator*(double factor){
	return Vector(this->x * factor, this->y * factor, this->z * factor);
}

double Vector::abs(){
	return(sqrt(x*x + y*y + z*z));
}

Vector Vector::cross(Vector b){
	return Vector((this->y*b.z) - (this->z*b.y), (this->z*b.x) - (this->x*b.z), (this->x*b.y) - (this->y*b.x));
}

Vector Vector::normalize(){
	double factor = 1/this->abs();
	return Vector(factor*x, factor*y, factor*z);
}

std::string Vector::toString(){
	std::stringstream out;

	out << this->x << " " << this->y << " " << this->z << std::endl;

	return out.str();
}


