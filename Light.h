#ifndef _Light_H
#define _Light_H

#include "Vector.h"
#include "Source.h"

class Light : public Source {

public:
	Vector position;

	Light() {
		position = Vector(0, 0, 0);
	}

	Light(Vector p) {
		position = p;
	}

	Vector getLightPosition() { return position; }

};

#endif
