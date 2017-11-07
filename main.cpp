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
#include "Colour.h"

using namespace std;

struct RGBType {
	double r;
	double g;
	double b;
};

void normalizeRGB(Colour& color) {
	color.red = (color.red > 255) ? 255 : (color.red < 0) ? 0 : color.red;
	color.green = (color.green > 255) ? 255 : (color.green < 0) ? 0 : color.green;
	color.blue = (color.blue > 255) ? 255 : (color.blue < 0) ? 0 : color.blue;
}

void savebmp(const char *filename, int w, int h, int dpi, RGBType *data) {
	FILE *f;
	int k = w*h;
	int s = 4 * k;
	int filesize = 54 + s;

	double factor = 39.375;
	int m = static_cast<int>(factor);

	int ppm = dpi*m;

	unsigned char bmpfileheader[14] = { 'B', 'M', 0, 0, 0, 0, 0, 0, 0, 0, 54, 0, 0, 0 };
	unsigned char bmpinfoheader[40] = { 40, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 24, 0 };

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

		unsigned char color[3] = { (int)floor(blue), (int)floor(green), (int)floor(red) };

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
	Vector campos(0, 0, 6);
	Vector look_at(Origin);
	Vector diff_btw(campos.x - look_at.x, campos.y - look_at.y, campos.z - look_at.z);
	Vector camdir = diff_btw.negative().normalize();
	Vector camright = Y.crossProduct(camdir).normalize();
	Vector camdown = camright.crossProduct(camdir);
	Camera camera(campos, camdir, camright, camdown);

	// model colors
	Colour color_white(255, 255, 255, 0);
	Colour color_black(0, 0, 0, 0);
	Colour color_red(255, 0, 0, 0);
	Colour color_green(0, 255, 0, 0);
	Colour color_blue(0, 0, 255, 0);

	vector<Object*> objects;

	// model objects
	Sphere sphere(Vector(0, 0, 0), 1, color_green);
	Sphere sphere2(Vector(-2, 0, 0), 0.5, color_red);
	Sphere sphere3(Vector(2, -0.5, 0), 0.75, color_blue);
	
	objects.push_back(&sphere);
	objects.push_back(&sphere2);
	objects.push_back(&sphere3);

	// model light sources
	Light light(Vector(0, 3, 5), color_white);
	vector<Source*> lights;
	lights.push_back(&light);

	// needed global variables
	double perspectiveX, perspectiveY;
	RGBType *pixels = new RGBType[numOfPixels];
	Colour pixel_color;
	double lightIntensity = 1;
	double material = 1;

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
				if (intersectValue == -1) {
					continue;
				}
				else {
					Vector intersection_position = cam_ray_origin.vectAdd(cam_ray_direction.vectMult(intersectValue));					// position of ray-sphere intersection
					Vector intersection_to_light_direction = light.getLightPosition().vectSub(intersection_position).normalize();		// vector from intersection position to light source	
					Vector intersection_to_camera_direction = camera.getCameraPosition().vectSub(intersection_position).normalize();	// 
					Vector normal = objects[i]->getNormalAt(intersection_position).normalize();
					Vector normalL = normal.vectMult(normal.dotProduct(intersection_to_light_direction));
					double angle = intersection_to_light_direction.dotProduct(normal);					// angle between light ray to the object and the normal of the object
					Vector s = normalL.vectSub(intersection_to_light_direction);
					Vector reflect = (normalL.vectMult(2)).vectMult(angle).vectSub(intersection_to_light_direction);
	
					pixel_color = objects[i]->colour.ColourScalar(angle).ColourScalar(lightIntensity).ColourScalar(material);
					pixel_color = pixel_color.ColourScalar(reflect.dotProduct(intersection_to_camera_direction));

					normalizeRGB(pixel_color);			// normalizes rgb values

					pixels[thisPixel].r = pixel_color.red;
					pixels[thisPixel].g = pixel_color.green;
					pixels[thisPixel].b = pixel_color.blue;
				}

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
