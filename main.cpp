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
bool inShadows(Vector intersection_position, Vector intersection_to_light_direction, vector <Object*> objects, int num) {
	bool shadowed = false;
	for (Object* obj : objects)
	{
		if (obj == objects[num]) {
			continue;
		}
		else {
			Ray shadow(intersection_position, intersection_to_light_direction);
			if (obj->intersect(shadow, intersection_position) > -1) {//Schattenfühler hits other Object
				shadowed = true;
			}
		}
	}
	return shadowed;
}

int main(int argc, char** argv) {
	std::cout << "rendering ..." << std::endl;

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
	Vector campos(0, 2, 10);
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
	Colour color_red(255, 0, 0, 0);
	Colour color_green(0, 255, 0, 0);
	Colour color_blue(0, 0, 255, 0);
	Colour colour_cyan(0, 255, 255, 0);

	vector<Object*> objects;
	// model objects
	Plane floor(Vector(0, -5, 0), Vector(0, 1, 0), colour_cyan);
	Sphere sphere(Vector(2, -1, 0), 0.3, color_blue);
	Sphere sphere2(Vector(0, 0, 0), 0.5, color_green);
	Sphere sphere3(Vector(-2, 1, 0), 1, color_red);
	//Triangle triangle1(Vector(12, 12, -10), Vector(-12, 12, -10), Vector(-12, -12, -10), colour_cyan);
	//Triangle triangle2(Vector(-12, 12, -10), Vector(-12, 12, -10), Vector(-12, -12, -10), color_red);
	Triangle ground1(Vector(-8, -4, 8), Vector(8, -4, 8), Vector(8, -4, -8), color_red);
	Triangle ground2(Vector(8, -4, -8), Vector(-8, -4, -8),  Vector(-8, -4, 8),  color_red);
	Triangle wallBack1(Vector(8, 8, -8), Vector(-8, -4, -8), Vector(-8, 8, -8), color_red);
	Triangle wallBack2(Vector(8, 8, -8), Vector(-8, -4, -8), Vector(8, -4, -8), color_red);
	
	objects.push_back(&ground1);
	objects.push_back(&ground2);
	objects.push_back(&wallBack1);
	objects.push_back(&wallBack2);
	//objects.push_back(&ground1);
	//objects.push_back(&floor);
	objects.push_back(&sphere);
	objects.push_back(&sphere2);
	objects.push_back(&sphere3);

	// model light sources
	Light light(Vector(0, 10, 0), color_white);
	vector<Source*> lights;
	lights.push_back(&light);

	// needed global variables
	double perspectiveX, perspectiveY;
	RGBType *pixels = new RGBType[numOfPixels];
	Colour ambient_lighting, diffuse_reflection, reflective_reflection, pixel_color;
	double lightIntensity = 1;
	double material = 1;

	for (int x = 0; x < imageWidth; x++) {
		for (int y = 0; y < imageHeight; y++) {
			int thisPixel = y * imageWidth + x;
			pixel_color = color_stonegrey;

			pixels[thisPixel].r = color_stonegrey.red;
			pixels[thisPixel].g = color_stonegrey.green;
			pixels[thisPixel].b = color_stonegrey.blue;

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
					Vector normal = objects[i]->getNormalAt(intersection_position, intersection_to_light_direction).normalize();											// normal at point of intersection
					double angle = normal.dotProduct(intersection_to_light_direction);													// angle between light ray to the object and the normal of the object								
					Vector reflect = (normal.vectMult(2)).vectMult(angle).vectSub(intersection_to_light_direction);						// reflection vector	

					// check for shadows
					if (!inShadows(intersection_position, intersection_to_light_direction, objects, i)) {
						diffuse_reflection = objects[i]->colour.ColourScalar(angle).ColourScalar(lightIntensity).ColourScalar(material);			// diffuse reflection
						diffuse_reflection.normalizeRGB();

						// make sure angle is within correct range (between 1 and 0)
						if((angle < 1 && angle > 0)){
							reflective_reflection = light.colour.ColourScalar(pow(reflect.dotProduct(intersection_to_camera_direction), 15));		// reflective reflection
							reflective_reflection.normalizeRGB();
						}		
				
						pixel_color = ambient_lighting.ColourAdd(diffuse_reflection).ColourAdd(reflective_reflection);								// addition of reflections and lightings
					}
					else {
						pixel_color = ambient_lighting;
					}

					pixel_color.normalizeRGB();

					pixels[thisPixel].r = pixel_color.red;
					pixels[thisPixel].g = pixel_color.green;
					pixels[thisPixel].b = pixel_color.blue;
				}
			}
		}
	}

	BMP::savebmp("scene.bmp", imageWidth, imageHeight, dpi, pixels);

	return 0;
}

