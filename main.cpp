#include "BMP\Bitmap.h"
#include "BMP\BMPHandler.h"

#include <iostream>

#include "Vector.h"

using namespace std;

class Ray{
public:
	Vector origin;
	Vector direction;
	Ray(Vector orig, Vector dir){
		origin = orig;
		direction = dir.normalize();
	}
};

class Sphere{
public:
	Vector center;
	double radius;
	Sphere(Vector c, double r){
		center = c;
		radius = r;
	}

	bool intersect(Ray ray){
		Vector m = center;
		Vector o = ray.origin;
		Vector om = o - m;
		double l = abs(om * ray.direction);
		double h2squared = pow(om.abs(), 2) - l*l;
		double hsquared = radius*radius - h2squared;

		if (h2squared <= radius*radius){
			if (pow(om.abs(), 2) >= radius*radius) //not inside
			{
				Vector S = o + ray.direction * (l - sqrt(hsquared));
				cout << S.toString() << endl;
			}
			else {
				Vector S = o + ray.direction * (l + sqrt(hsquared));
				cout << S.toString() << endl;
			}
			return true;
		}
		return false;
	}
};

int main(int argc, char** argv)
{
	int imageHeight = 1000;
	int imageWidth = 1000;
	Sphere sphere(Vector(50, 50, -50), 20);
	BYTE * Buffer = new BYTE[3 * imageHeight * imageWidth];

	for (int height = 1; height <= imageHeight; height++)
	{
		for (int width = 1; width <= imageWidth; width++)
		{
			Ray ray(Vector(50, 50, 50), Vector(width, height, 0) - Vector(50, 50, 50) );
			//Ray ray(Vector(width, height, 0), Vector(0, 0, 1));
			
			if (sphere.intersect(ray)){
				Buffer[(height*width * 3)] = 0;
				Buffer[(height*width * 3) +1] = 0;
				Buffer[(height*width * 3) + 2] = 255;
			}
			else{
				Buffer[(height*width * 3)] = 0;
				Buffer[(height*width * 3)+1] = 0;
				Buffer[(height*width * 3)+2] = 0;
			}
		}
	}




	const char* imagesource = "image.bmp";
	BMPHandler::getInstance().Load24bitBMP(imagesource, 1, "image");
	Bitmap * image = BMPHandler::getInstance().findBitmapByKey("image");

	image->Buffer = Buffer;
	BMPHandler::getInstance().SaveBMP(image, "newimage.bmp", imageHeight, imageWidth, "image");
	system("pause");
	return 0;
}