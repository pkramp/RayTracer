#ifndef _Light_H
#define _Light_H

#include "Vector.h"
#include "Source.h"
#include "Colour.h"

class Light : public Source {

public:
	Vector position;
	Colour colour;

	Light() {
		position = Vector(0, 0, 0);
		colour.red = 255;
		colour.green = 255;
		colour.blue = 255;
		colour.special = 0;
	}

	Light(Vector p, Colour c) {
		position = p;
		colour.red = c.red;
		colour.green = c.green;
		colour.blue = c.blue;
		colour.special = c.special;
	}

	Vector getLightPosition() { return position; }

};

#endif
