#ifndef _Colour_H
#define _Colour_H

class Colour {

public:
	double red, green, blue, special, reflect, transparency;

	Colour() {
		red = 0.0;
		green = 0.0;
		blue = 0.0;
		special = 0.0;
		reflect = 0.0;
		transparency = 0.0;
	}

	Colour(double r, double g, double b, double s, double ref, double transp = 0) {
		red = r;
		green = g;
		blue = b;
		special = s;
		reflect = ref;
		transparency = transp;
	}

	double getColourRed() { return red; }
	double getColourGreen() { return green; }
	double getColourBlue() { return blue; }
	double getColourSpecial() { return special; }
	double getColourReflect() { return reflect; }
	double getColourTransparency() { return transparency; }

	double setColourRed(double redValue) { red = redValue; }
	double setColourGreen(double greenValue) { green = greenValue; }
	double setColourBlue(double blueValue) { blue = blueValue; }
	double setColourSpecial(double specialValue) { special = specialValue; }
	double setColourReflect(double reflectValue) { reflect = reflectValue; }


	double brightness() {
		return ((red + green + blue) / 3);
	}

	Colour ColourScalar(double scalar) {
		return Colour(red*scalar, green*scalar, blue*scalar, special, reflect);
	}

	Colour ColourAdd(Colour col) {
		return Colour(red + col.getColourRed(), green + col.getColourGreen(), blue + col.getColourBlue(), special, reflect);
	}

	Colour ColourMultiply(Colour col) {
		return Colour(red*col.getColourRed(), green*col.getColourGreen(), blue*col.getColourBlue(), special, reflect);
	}

	Colour ColourAverage(Colour col) {
		return Colour((red + col.getColourRed()) / 2, (green + col.getColourGreen()) / 2, (blue + col.getColourBlue()) / 2, special, reflect);
	}


	void normalizeRGB() {
		red = (red > 255) ? 255 : (red < 0) ? 0 : red;
		green = (green > 255) ? 255 : (green < 0) ? 0 : green;
		blue = (blue > 255) ? 255 : (blue < 0) ? 0 : blue;
	}
};

#endif
