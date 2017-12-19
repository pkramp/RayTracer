#ifndef Vector2_H
#define Vector2_H

class Vector2 {

public:
	double x, y;

	Vector2() {
		this->x = 0;
		this->y = 0;
	}

	Vector2(const Vector2& copy) {
		this->x = copy.x;
		this->y = copy.y;
	}

	Vector2(double x, double y) {
		this->x = x;
		this->y = y;
	}
};

#endif Vector2_H