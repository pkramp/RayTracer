#ifndef _Colour_H
#define _Colour_H

class Colour {

public:
	double red, green, blue, special;

	Colour() {
		red = 0.5;
		green = 0.5;
		blue = 0.5;
		special = 0.0;
	}

	Colour(double r, double g, double b, double s) {
		red = r;
		green = g;
		blue = b;
		special = s;
	}

	double getColourRed() { return red; }
	double getColourGreen() { return green; }
	double getColourBlue() { return blue; }
	double getColourSpecial() { return special; }

	double setColourRed(double redValue) { red = redValue; }
	double setColourGreen(double greenValue) { green = greenValue; }
	double setColourBlue(double blueValue) { blue = blueValue; }
	double setColourSpecial(double specialValue) { special = specialValue; }


	double brightness() {
		return(red + green + blue) / 3;
	}

	Colour ColourScalar(double scalar) {
		return Colour(red*scalar, green*scalar, blue*scalar, special);
	}

	Colour ColourAdd(Colour col) {
		return Colour(red + col.getColourRed(), green + col.getColourGreen(), blue + col.getColourBlue(), special);
	}

	Colour ColourMultiply(Colour col) {
		return Colour(red*col.getColourRed(), green*col.getColourGreen(), blue*col.getColourBlue(), special);
	}

	Colour ColourAverage(Colour col) {
		return Colour((red + col.getColourRed()) / 2, (green + col.getColourGreen()) / 2, (blue + col.getColourBlue()) / 2, special);
	}
};

#endif
