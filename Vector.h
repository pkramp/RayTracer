#include <string>
#include <sstream>

class Vector{

public:
	Vector();
	Vector(float, float, float);

	std::string toString();
	Vector operator+(const Vector other) const;
	Vector operator-(const Vector other) const;
	Vector operator / (float d) const { return Vector(x / d, y / d, z / d); }
	float operator*(const Vector other) const;
	Vector operator*(const float factor) const;
	bool operator==(const Vector& other) const;
	bool operator!=(const Vector& other) const;
	float abs()const;
	//float scale(Vector);
	Vector cross(Vector);
	Vector normalize() const;


//private:
	float x, y, z;
};