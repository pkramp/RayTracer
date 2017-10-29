#include <fstream>
#include <cmath>
#include <iostream>
#include <vector>
#include <time.h>

#include "Vector.h"
#include "Ray.h"
#include "Sphere.h"
#include "Camera.h"
#include "Light.h"
#include "Object.h"
#include "Source.h"

using namespace std;

struct RGBType {
	double r;
	double g;
	double b;
};

void normalizeRGB(Vector& color) {
	color.x = (color.x > 255) ? 255 : (color.x < 0) ? 0 : color.x;
	color.y = (color.y > 255) ? 255 : (color.y < 0) ? 0 : color.y;
	color.z = (color.z > 255) ? 255 : (color.z < 0) ? 0 : color.z;
}

void savebmp(const char *filename, int w, int h, int dpi, RGBType *data) {
	FILE *f;
	int k = w*h;
	int s = 4 * k;
	int filesize = 54 + s;

	double factor = 39.375;
	int m = static_cast<int>(factor);

	int ppm = dpi*m;

	unsigned char bmpfileheader[14] = { 'B','M', 0,0,0,0, 0,0,0,0, 54,0,0,0 };
	unsigned char bmpinfoheader[40] = { 40,0,0,0, 0,0,0,0, 0,0,0,0, 1,0,24,0 };

	bmpfileheader[2] = (unsigned char)(filesize);
	bmpfileheader[3] = (unsigned char)(filesize >> 8);
	bmpfileheader[4] = (unsigned char)(filesize >> 16);
	bmpfileheader[5] = (unsigned char)(filesize >> 24);

	bmpinfoheader[4] = (unsigned char)(w);
	bmpinfoheader[5] = (unsigned char)(w >> 8);
	bmpinfoheader[6] = (unsigned char)(w >> 16);
	bmpinfoheader[7] = (unsigned char)(w >> 24);

	bmpinfoheader[8] = (unsigned char)(h);
	bmpinfoheader[9] = (unsigned char)(h >> 8);
	bmpinfoheader[10] = (unsigned char)(h >> 16);
	bmpinfoheader[11] = (unsigned char)(h >> 24);

	bmpinfoheader[21] = (unsigned char)(s);
	bmpinfoheader[22] = (unsigned char)(s >> 8);
	bmpinfoheader[23] = (unsigned char)(s >> 16);
	bmpinfoheader[24] = (unsigned char)(s >> 24);

	bmpinfoheader[25] = (unsigned char)(ppm);
	bmpinfoheader[26] = (unsigned char)(ppm >> 8);
	bmpinfoheader[27] = (unsigned char)(ppm >> 16);
	bmpinfoheader[28] = (unsigned char)(ppm >> 24);

	bmpinfoheader[29] = (unsigned char)(ppm);
	bmpinfoheader[30] = (unsigned char)(ppm >> 8);
	bmpinfoheader[31] = (unsigned char)(ppm >> 16);
	bmpinfoheader[32] = (unsigned char)(ppm >> 24);

	fopen_s(&f, filename, "wb");

	fwrite(bmpfileheader, 1, 14, f);
	fwrite(bmpinfoheader, 1, 40, f);

	for (int i = 0; i < k; i++) {
		RGBType rgb = data[i];

		double red = (data[i].r);
		double green = (data[i].g);
		double blue = (data[i].b);

		unsigned char color[3] = { (int)floor(blue),(int)floor(green),(int)floor(red) };

		fwrite(color, 1, 3, f);
	}

	fclose(f);
}

int main(int argc, char** argv) {
	cout << "rendering ..." << endl;

	clock_t t1, t2;
	t1 = clock();

	// images properties
	int imageWidth = 640;
	int imageHeight = 480;
	int dpi = 72;
	int numOfPixels = imageHeight * imageWidth;
	double aspectratio = (double)imageWidth / (double)imageHeight;
	
	// global coordinates
	Vector Origin(0, 0, 0);
	Vector X(1, 0, 0);
	Vector Y(0, 1, 0);
	Vector Z(0, 0, 1);

	// model the camera
	Vector campos(0, 0, 5);
	Vector look_at(Origin);
	Vector diff_btw(campos.x - look_at.x, campos.y - look_at.y, campos.z - look_at.z);
	Vector camdir = diff_btw.negative().normalize();
	Vector camright = Y.crossProduct(camdir).normalize();
	Vector camdown = camright.crossProduct(camdir);
	Camera camera(campos, camdir, camright, camdown);

	// model a sphere
	Sphere sphere(Vector(0, 0, 0), 1);
	vector<Object*> objects;
	objects.push_back(dynamic_cast<Object*>(&sphere));

	// model colors
	Vector color_white(255, 255, 255);
	Vector color_black(0, 0, 0);
	Vector color_red(255, 0, 0);
	Vector color_green(0, 255, 0);
	Vector color_blue(0, 0, 255);
	
	// model light source
	Light light(Vector(0, 5, 0));
	vector<Source*> lights;
	lights.push_back(dynamic_cast<Source*>(&light));

	// needed global variables
	double perspectiveX, perspectiveY;	
	RGBType *pixels = new RGBType[numOfPixels];
	Vector pixel_color;
	double lightIntensity = 0.8;

	for (int x = 0; x < imageWidth; x++) {
		for (int y = 0; y < imageHeight; y++) {
			int thisPixel = y * imageWidth + x;

			// create camera ray
			if (imageWidth > imageHeight) {
				// the image is wider than it is tall
				perspectiveX = ((x + 0.5) / imageWidth)*aspectratio - (((imageWidth - imageHeight) / (double)imageHeight) / 2);
				perspectiveY = ((imageHeight - y) + 0.5) / imageHeight;
			}
			else if (imageHeight > imageWidth) {
				// the imager is taller than it is wide
				perspectiveX = (x + 0.5) / imageWidth;
				perspectiveY = (((imageHeight - y) + 0.5) / imageHeight) / aspectratio - (((imageHeight - imageWidth) / (double)imageWidth) / 2);
			}
			else {
				// the image is square
				perspectiveX = (x + 0.5) / imageWidth;
				perspectiveY = ((imageHeight - y) + 0.5) / imageHeight;
			}

			Vector cam_ray_origin = camera.getCameraPosition();
			Vector cam_ray_direction = camdir.vectAdd(camright.vectMult(perspectiveX - 0.5).vectAdd(camdown.vectMult(perspectiveY - 0.5))).normalize();

			Ray cameraRay(cam_ray_origin, cam_ray_direction);

			vector<double> intersections;
			double intersectValue;

			for (int i = 0; i < objects.size(); i++) {
				intersectValue = objects[i]->intersect(cameraRay);
				intersections.push_back(intersectValue);
			}

			if(intersectValue == -1) {
				pixels[thisPixel].r = 0;
				pixels[thisPixel].g = 0;
				pixels[thisPixel].b = 0;
			}
			else {
				Vector intersection_position = cam_ray_origin.vectAdd(cam_ray_direction.vectMult(intersectValue));
				Vector intersection_to_light_direction = light.getLightPosition().vectSub(intersection_position).normalize();
				Vector normal = sphere.getNormalAt(intersection_position).normalize();

				double angle = intersection_to_light_direction.dotProduct(normal);			// angle between light ray to the object and the normal of the object
				pixel_color = color_white.vectMult(angle).vectMult(lightIntensity);						

				normalizeRGB(pixel_color);			// normalizes rgb values

				pixels[thisPixel].r = pixel_color.x;
				pixels[thisPixel].g = pixel_color.y;
				pixels[thisPixel].b = pixel_color.z;

				// old functionality which however does not work properly
				/*Ray intersectionRay(intersection_position, intersecting_to_light_direction);

				vector<double> shadowIntersections;
				double shadowIntersection;

				for (int i = 0; i < objects.size(); i++) {
					shadowIntersection = objects[i]->intersect(intersectionRay);
					shadowIntersections.push_back(shadowIntersection);
				}

				if (shadowIntersection == -1 || shadowIntersection == intersectValue) {
					pixels[thisPixel].r = 255;
					pixels[thisPixel].g = 255;
					pixels[thisPixel].b = 255;
				}
				else if (shadowIntersection != -1 && shadowIntersection != intersectValue) {
					pixels[thisPixel].r = 128;
					pixels[thisPixel].g = 128;
					pixels[thisPixel].b = 128;
				}*/
			}
		}		
	}

	savebmp("scene.bmp", imageWidth, imageHeight, dpi, pixels);
	
	t2 = clock();
	double time = ((double)t2 - (double)t1) / 1000;

	cout << "Image rendered in " << time << " seconds." << endl;

	system("Pause");
	return 0;
}
