#ifndef _SOURCE_H
#define _SOURCE_H

#include "Vector.h"

class Source {
public:

	Source() {};

	virtual Vector getLightPosition() { return Vector(0, 0, 0); }

};

#endif