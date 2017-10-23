#include "BMP\Bitmap.h"
#include "BMP\BMPHandler.h"

#include <fstream>
#include <cmath>
#include <iostream>

#include "Vector.h"

using namespace std;

class Ray {
public:
	Vector origin;
	Vector direction;
	Ray(Vector orig, Vector dir) {
		origin = orig;
		direction = dir.normalize();
	}
};

class Sphere {
public:
	Vector center;
	float radius;
	Sphere(Vector c, float r) {
		center = c;
		radius = r;
	}

	bool intersect(Ray ray, Vector& S) {
		Vector m = center;
		Vector o = ray.origin;
		Vector om = o - m;
		float l = /*abs*/(om * ray.direction);
		float h2squared = pow(om.abs(), 2) - l*l;
		float hsquared = radius*radius - h2squared;

		if (h2squared <= radius*radius) {
			if (pow(om.abs(), 2) >= radius*radius) //not inside
			{
				//if (l < 0)
					//return false;
				S = o + ray.direction * (l - sqrt(hsquared));
				//cout << S.toString() << endl;
			}
			else {
				S = o + ray.direction * (l + sqrt(hsquared));
				//cout << S.toString() << endl;
			}
			return true;
		}
		return false;
	}
};

int main(int argc, char** argv)
{
	int imageHeight = 500;
	int imageWidth = 500;
	Vector eye(250, 250, 1);
	Vector light(250, 400, 400);
	Sphere sphere(Vector(250, 250, 400), 80.5);
	Vector S;

	std::ofstream out("out.ppm");
	out << "P3\n" << imageWidth << ' ' << imageHeight << ' ' << "255\n";
	for (int height = 0; height < imageHeight; height++)
	{
		for (int width = 0; width < imageWidth; width++)
		{
			Ray ray(eye, Vector((float)height, (float)width, 300) - eye);
			//Ray ray(Vector(width, height, 0), Vector(0, 0, 1));

			if (sphere.intersect(ray, S)) {
				Ray shadow(S, light - S);
				Vector S2;
				if (sphere.intersect(shadow, S2) && S2 != S) {
					out << (int)0 << ' '
						<< (int)0 << ' '
						<< (int)0 << '\n';
				}
				else {
					out << (int)255 << ' '
						<< (int)255 << ' '
						<< (int)255 << '\n';
				}
			}
			else {
				out << (int)0 << ' '
					<< (int)0 << ' '
					<< (int)0 << '\n';
			}
		}
	}
	return 0;
}
