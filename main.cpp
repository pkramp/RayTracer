#include <fstream>
#include <cmath>
#include <iostream>
#include <vector>

#include "Vector.h"
#include "Ray.h"
#include "Sphere.h"
#include "Camera.h"

using namespace std;

struct RGBType {
	double r;
	double g;
	double b;
};

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
	int imageWidth = 640;
	int imageHeight = 480;
	int dpi = 72;
	int numOfPixels = imageHeight * imageWidth;
	double aspectratio = (double)imageWidth / (double)imageHeight;
	RGBType *pixels = new RGBType[numOfPixels];
	
	Vector Origin(0, 0, 0);
	Vector X(1, 0, 0);
	Vector Y(0, 1, 0);
	Vector Z(0, 0, 1);

	Vector campos(0, 0, 5);
	Vector look_at(Origin);
	Vector diff_btw(campos.x - look_at.x, campos.y - look_at.y, campos.z - look_at.z);

	Vector camdir = diff_btw.negative().normalize();
	Vector camright = Y.crossProduct(camdir).normalize();
	Vector camdown = camright.crossProduct(camdir);
	Camera scene_cam(campos, camdir, camright, camdown);
	double xamnt, yamnt;

	Sphere sphere(Vector(0, 0, 0), 1.5);
	Vector light(0, 3, 0);

	for (int x = 0; x < imageWidth; x++) {
		for (int y = 0; y < imageHeight; y++) {
			int thisPixel = y * imageWidth + x;

			// create camera ray
			if (imageWidth > imageHeight) {
				// the image is wider than it is tall
				xamnt = ((x + 0.5) / imageWidth)*aspectratio - (((imageWidth - imageHeight) / (double)imageHeight) / 2);
				yamnt = ((imageHeight - y) + 0.5) / imageHeight;
			}
			else if (imageHeight > imageWidth) {
				// the imager is taller than it is wide
				xamnt = (x + 0.5) / imageWidth;
				yamnt = (((imageHeight - y) + 0.5) / imageHeight) / aspectratio - (((imageHeight - imageWidth) / (double)imageWidth) / 2);
			}
			else {
				// the image is square
				xamnt = (x + 0.5) / imageWidth;
				yamnt = ((imageHeight - y) + 0.5) / imageHeight;
			}

			Vector cam_ray_origin = scene_cam.getCameraPosition();
			Vector cam_ray_direction = camdir.vectAdd(camright.vectMult(xamnt - 0.5).vectAdd(camdown.vectMult(yamnt - 0.5))).normalize();

			Ray cameraRay(cam_ray_origin, cam_ray_direction);

			double intersectValue = sphere.intersect(cameraRay);

			if(intersectValue == -1) {
				pixels[thisPixel].r = 0;
				pixels[thisPixel].g = 0;
				pixels[thisPixel].b = 0;
			}
			else {
				Vector intersection_position = cam_ray_origin.vectAdd(cam_ray_direction.vectMult(intersectValue));
				Vector intersecting_to_light_direction = light.vectSub(intersection_position);

				Ray intersectionRay(intersection_position, intersecting_to_light_direction);

				double shadowIntersection = sphere.intersect(intersectionRay);

				if (shadowIntersection == -1 || shadowIntersection == intersectValue) {
					pixels[thisPixel].r = 255;
					pixels[thisPixel].g = 255;
					pixels[thisPixel].b = 255;
				}
				else
					pixels[thisPixel].r = 128;
					pixels[thisPixel].g = 128;
					pixels[thisPixel].b = 128;
			}
		}		
	}
	savebmp("scene.bmp", imageWidth, imageHeight, dpi, pixels);
	
	return 0;
}
