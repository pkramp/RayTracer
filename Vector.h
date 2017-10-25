#ifndef VECTOR_H
#define VECTOR_H


#include <string>
#include <sstream>

class Vector{

public:
	Vector();
	Vector(double, double, double);

	std::string toString();
	Vector operator+(const Vector other) const;
	Vector operator-(const Vector other) const;
	Vector operator / (double d) const { return Vector(x / d, y / d, z / d); }
	double operator*(const Vector other) const;
	Vector operator*(const double factor) const;
	bool operator==(const Vector& other) const;
	bool operator!=(const Vector& other) const;
	double abs()const;
	//double scale(Vector);
	Vector crossProduct(Vector);
	Vector normalize() const;
	Vector negative();

//private:
	double x, y, z;
};
#endif // !VECTOR_H