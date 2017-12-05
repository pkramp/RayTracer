#include <fstream>
#include <cmath>
#include <iostream>
#include <vector>
#include <time.h>

#include "Vector.h"
#include "BMP.h"
#include "Ray.h"
#include "Sphere.h"
#include "Camera.h"
#include "Light.h"
#include "Triangle.h"
#include "Object.h"
#include "Source.h"
#include "Colour.h"
#include "Plane.h"

using namespace std;

static const int aadepth = 4;

bool inShadows(Vector intersection_position, Vector intersection_to_light_direction, vector <Object*> objects, int num, vector <Source*> lights) {
	bool shadowed = false;
	Vector actualIntersection = intersection_position;
	for (Object* obj : objects)
	{
		if (obj == objects[num]) {
			continue;
		}
		else {
			Ray shadow(intersection_position, intersection_to_light_direction);
			if (obj->intersect(shadow, intersection_position) > -1) {	//Schattenfühler hits other Object
				if (lights[0]->getLightPosition().vectSub(actualIntersection).magnitude() > intersection_position.vectSub(actualIntersection).magnitude()) {
					shadowed = true;
				}
			}
		}
	}
	return shadowed;
}

int main(int argc, char** argv) {
	std::cout << "rendering ..." << std::endl;

	clock_t t1, t2;
	t1 = clock();

	// images properties
	int imageWidth = 1024;
	int imageHeight = 856;
	int dpi = 72;
	int numOfPixels = imageHeight * imageWidth;
	double aspectratio = (double)imageWidth / (double)imageHeight;

	// global coordinates
	Vector Origin(0, 0, 0);
	Vector X(1, 0, 0);
	Vector Y(0, 1, 0);
	Vector Z(0, 0, 1);

	// model the camera
	Vector campos(0, 6, 15);
	Vector look_at(Origin);
	Vector diff_btw(campos.x - look_at.x, campos.y - look_at.y, campos.z - look_at.z);
	Vector camdir = diff_btw.negative().normalize();
	Vector camright = Y.crossProduct(camdir).normalize();
	Vector camdown = camright.crossProduct(camdir);
	Camera camera(campos, camdir, camright, camdown);

	// model colors
	Colour color_white(255, 255, 255, 0);
	Colour color_black(0, 0, 0, 0);
	Colour color_stonegrey(128, 128, 128, 0);
	Colour color_winered(160, 0, 32, 0);
	Colour color_green(0, 255, 0, 0);
	Colour color_blue(65, 105, 255, 0);
	Colour colour_cyan(0, 255, 255, 0);

	vector<Object*> objects;
	// model objects
	Sphere sphere(Vector(2, -1, 0), 0.3, color_black);
	Sphere sphere2(Vector(0, 0, 0), 0.5, color_green);
	Sphere sphere3(Vector(-2, 1, 0), 1, colour_cyan);
	Triangle ground1(Vector(-8, -4, 8), Vector(8, -4, 8), Vector(8, -4, -8), color_winered);
	Triangle ground2(Vector(8, -4, -8), Vector(-8, -4, -8),  Vector(-8, -4, 8), color_winered);
	//Triangle ceiling1(Vector(-8, 8, 8), Vector(8, 8, 8), Vector(8, 8, -8), color_blue);
	//Triangle ceiling2(Vector(8, 8, -8), Vector(-8, 8, -8), Vector(-8, 8, 8), color_blue);
	Triangle wallBack1(Vector(-8, -4, -8), Vector(8, 8, -8),  Vector(-8, 8, -8), color_stonegrey);
	Triangle wallBack2(Vector(8, 8, -8),  Vector(8, -4, -8), Vector(-8, -4, -8), color_stonegrey);
	Triangle wallLeft1(Vector(8, 8, -8), Vector(8, -4, -8), Vector(8, 8, 8), color_winered);
	Triangle wallLeft2(Vector(8, 8, 8), Vector(8, -4, -8), Vector(8, -4, 8), color_winered);
	Triangle wallRight1(Vector(-8, 8, -8), Vector(-8, -4, -8), Vector(-8, 8, 8), color_blue);
	Triangle wallRight2(Vector(-8, 8, 8), Vector(-8, -4, -8), Vector(-8, -4, 8), color_blue);
	
	objects.push_back(&ground1);
	objects.push_back(&ground2);	
	objects.push_back(&wallBack1);
	objects.push_back(&wallBack2);
	objects.push_back(&wallLeft1);
	objects.push_back(&wallLeft2);
	objects.push_back(&wallRight1);
	objects.push_back(&wallRight2);
	objects.push_back(&sphere);
	objects.push_back(&sphere2);
	objects.push_back(&sphere3);
	//objects.push_back(&ceiling1);
	//objects.push_back(&ceiling2);

	// model light sources
	Light light(Vector(0, 10, 0), color_white);
	vector<Source*> lights;
	lights.push_back(&light);

	// needed global variables
	int thisPixel, aa_index;
	double perspectiveX, perspectiveY;
	RGBType *pixels = new RGBType[numOfPixels];
	Colour ambient_lighting, diffuse_reflection, reflective_reflection, pixel_color;
	double lightIntensity = 0.7;
	double material = 1;
	double tempRed, tempGreen, tempBlue;

	for (int x = 0; x < imageWidth; x++) {
		for (int y = 0; y < imageHeight; y++) {
			thisPixel = y * imageWidth + x;

			double tempRed[aadepth*aadepth];
			double tempGreen[aadepth*aadepth];
			double tempBlue[aadepth*aadepth];

			for (int aax = 0; aax < aadepth; aax++) {
				for (int aay = 0; aay < aadepth; aay++) {

					srand(time(0));

					aa_index = aay*aadepth + aax;

					tempRed[aa_index] = 128;
					tempGreen[aa_index] = 128;
					tempBlue[aa_index] = 128;

					// create camera ray
					if (aadepth <= 1) {			// no anti aliasing
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
					}
					else {			// using anti aliasing
						if (imageWidth > imageHeight) {
							// the image is wider than it is tall
							perspectiveX = ((x + (double)aax / ((double)aadepth - 1)) / imageWidth)*aspectratio - (((imageWidth - imageHeight) / (double)imageHeight) / 2);
							perspectiveY = ((imageHeight - y) + (double)aax / ((double)aadepth - 1)) / imageHeight;
						}
						else if (imageHeight > imageWidth) {
							// the imager is taller than it is wide
							perspectiveX = (x + (double)aax / ((double)aadepth - 1)) / imageWidth;
							perspectiveY = (((imageHeight - y) + (double)aax / ((double)aadepth - 1)) / imageHeight) / aspectratio - (((imageHeight - imageWidth) / (double)imageWidth) / 2);
						}
						else {
							// the image is square
							perspectiveX = (x + (double)aax / ((double)aadepth - 1)) / imageWidth;
							perspectiveY = ((imageHeight - y) + (double)aax / ((double)aadepth - 1)) / imageHeight;
						}
					}

					Vector cam_ray_origin = camera.getCameraPosition();
					Vector cam_ray_direction = camdir.vectAdd(camright.vectMult(perspectiveX - 0.5).vectAdd(camdown.vectMult(perspectiveY - 0.5))).normalize();

					Ray cameraRay(cam_ray_origin, cam_ray_direction);

					Vector intersection_position;
			
					for (unsigned int i = 0; i < objects.size(); i++) {
						if (objects[i]->intersect(cameraRay, intersection_position) == -1) {
							// ray does not intersect the object
							continue;
						}
						else {
							// ray intersects the object

							// initialize different lighting values
							diffuse_reflection = Colour(0, 0, 0, 0);
							reflective_reflection = Colour(0, 0, 0, 0);
							ambient_lighting = objects[i]->colour.ColourScalar(0.1);															// ambient lighting

							// calculate lighting and reflection
							Vector intersection_to_light_direction = light.getLightPosition().vectSub(intersection_position).normalize();		// vector from intersection position to light source	
							Vector intersection_to_camera_direction = camera.getCameraPosition().vectSub(intersection_position).normalize();	// vector from intersection to camera position
							Vector normal = objects[i]->getNormalAt(intersection_position, intersection_to_light_direction).normalize();		// normal at point of intersection
							double angle = normal.dotProduct(intersection_to_light_direction);													// angle between light ray to the object and the normal of the object								
							Vector reflect = (normal.vectMult(2)).vectMult(angle).vectSub(intersection_to_light_direction);						// reflection vector	

							// check for shadows
							if (!inShadows(intersection_position, intersection_to_light_direction, objects, i, lights)) {
								diffuse_reflection = objects[i]->colour.ColourScalar(angle).ColourScalar(lightIntensity).ColourScalar(material);			// diffuse reflection
								diffuse_reflection.normalizeRGB();

								// make sure angle is within correct range (between 1 and 0)
								if ((angle < 1 && angle > 0)) {
									reflective_reflection = light.colour.ColourScalar(pow(reflect.dotProduct(intersection_to_camera_direction), 100));		// reflective reflection
									reflective_reflection.normalizeRGB();
								}

								pixel_color = ambient_lighting.ColourAdd(diffuse_reflection).ColourAdd(reflective_reflection);								// addition of reflections and lightings
							}
							else {
								pixel_color = ambient_lighting;
							}

							pixel_color.normalizeRGB();

							tempRed[aa_index] = pixel_color.red;
							tempGreen[aa_index] = pixel_color.green;
							tempBlue[aa_index] = pixel_color.blue;
						}
					}

					// average the pixel color
					double totalRed = 0;
					double totalGreen = 0;
					double totalBlue = 0;

					for (int iRed = 0; iRed < aadepth*aadepth; iRed++) {
						totalRed = totalRed + tempRed[iRed];
					}
					for (int iGreen = 0; iGreen < aadepth*aadepth; iGreen++) {
						totalGreen = totalGreen + tempGreen[iGreen];
					}
					for (int iBlue = 0; iBlue < aadepth*aadepth; iBlue++) {
						totalBlue = totalBlue + tempBlue[iBlue];
					}

					double avgRed = totalRed / (aadepth*aadepth);
					double avgGreen = totalGreen / (aadepth*aadepth);
					double avgBlue = totalBlue / (aadepth*aadepth);

					pixels[thisPixel].r = avgRed;
					pixels[thisPixel].g = avgGreen;
					pixels[thisPixel].b = avgBlue;
				}
			}
		}
	}

	BMP::savebmp("scene.bmp", imageWidth, imageHeight, dpi, pixels);

	t2 = clock();
	float diff = ((float)t2 - (float)t1) / 1000;

	cout << "rendered " << imageWidth << " x " << imageHeight << " image in " << diff << " seconds, using AA depth of " << aadepth << endl;

	system("Pause");

	return 0;
}

