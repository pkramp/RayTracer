#include <string>
#include <sstream>

class Vector{

public:
	Vector();
	Vector(double, double, double);

	std::string toString();
	Vector operator+(Vector other);
	Vector operator-(Vector other);
	double operator*(Vector other);
	Vector operator*(double factor);
	double abs();
	//double scale(Vector);
	Vector cross(Vector);
	Vector normalize();


private:
	double x, y, z;
};