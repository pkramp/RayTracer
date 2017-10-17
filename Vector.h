#include <string>
#include <sstream>

class Vector{

public:
	Vector();
	Vector(double, double, double);

	std::string toString();
	Vector operator+(Vector other);
	Vector operator-(Vector other);
	Vector operator*(Vector other);
	Vector operator*(double factor);
	double scale(Vector);


private:
	double x, y, z;
};