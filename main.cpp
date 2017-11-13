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
#include "Object.h"
#include "Source.h"
#include "Colour.h"

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
			if (obj->intersect(shadow) > -1) {//Schattenfühler hits other Object
				shadowed = true;
			}
		}
	}
	return shadowed;
}

int main(int argc, char** argv) {
	cout << "rendering ..." << endl;

	clock_t t1;
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
	Colour colour_yellow(0, 255, 255, 0);

	vector<Object*> objects;
	// model objects
	Sphere sphere(Vector(2, -1, 0), 0.3, color_blue);
	Sphere sphere2(Vector(0, 0, 0), 0.5, color_green);
	Sphere sphere3(Vector(-2, 1, 0), 1, color_red);

	objects.push_back(&sphere);
	objects.push_back(&sphere2);
	objects.push_back(&sphere3);


	// model light sources
	Light light(Vector(8, -3, -1 ), color_white);
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
			pixel_color = color_black;

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

			double intersectValue;

			for (unsigned int i = 0; i < objects.size(); i++) {
				intersectValue = objects[i]->intersect(cameraRay);
				if (intersectValue == -1) {
					// ray does not intersect the sphere
					continue;
				}
				else {
					// ray intersects the sphere
					Vector intersection_position = cam_ray_origin.vectAdd(cam_ray_direction.vectMult(intersectValue));					// position of ray-sphere intersection
					Vector intersection_to_light_direction = light.getLightPosition().vectSub(intersection_position).normalize();		// vector from intersection position to light source	

					Vector intersection_to_camera_direction = camera.getCameraPosition().vectSub(intersection_position).normalize();	// vector from intersection to camera position
					Vector normal = objects[i]->getNormalAt(intersection_position).normalize();											// normal at point of intersection
					//Vector normalL = normal.vectMult(normal.dotProduct(intersection_to_light_direction)).normalize();								// vector of projection of light ray onto normal
					double angle = normal.dotProduct(intersection_to_light_direction);
					//double angle = intersection_to_light_direction.dotProduct(normal);											// angle between light ray to the object and the normal of the object
					//Vector s = normalL.vectSub(intersection_to_light_direction);												// distance from normal lenght projection to reflection vector
					
					Vector reflect = (normal.vectMult(2)).vectMult(angle).vectSub(intersection_to_light_direction);// .normalize();			// reflection vector	
					//Vector reflect = normal.vectMult(angle).vectMult(2).vectSub(intersection_to_light_direction);// .normalize();
					ambient_lighting = objects[i]->colour.ColourScalar(0.1);																	// ambient lighting
					if (!inShadows(intersection_position, intersection_to_light_direction, objects, i)) {
						diffuse_reflection = objects[i]->colour.ColourScalar(angle).ColourScalar(lightIntensity).ColourScalar(material);			// diffuse reflection
						diffuse_reflection.normalizeRGB();
						//if(objects[i]->intersect(intersection_to_light_direction))
						//if(diffuse_reflection.blue > ambient_lighting.blue || diffuse_reflection.red > ambient_lighting.red || diffuse_reflection.green > ambient_lighting.green)
						if(diffuse_reflection.brightness() > ambient_lighting.brightness())
							reflective_reflection = light.colour.ColourScalar(pow(reflect.dotProduct(intersection_to_camera_direction), 10));			// reflective reflection		
							
						//reflective_reflection = light.colour.ColourScalar(pow(reflect.dotProduct(intersection_to_camera_direction), 100));
						reflective_reflection.normalizeRGB();
						pixel_color = ambient_lighting.ColourAdd(diffuse_reflection).ColourAdd(reflective_reflection);								// addition of reflections and lightings
					}
					else {
						pixel_color = ambient_lighting;
					}
						/*if (pixel_color.red > 200)
							cout << "hier";*/
					pixel_color.normalizeRGB();

					pixels[thisPixel].r = pixel_color.red;
					pixels[thisPixel].g = pixel_color.green;
					pixels[thisPixel].b = pixel_color.blue;
				}
			}
		}
	}

	//cout << "Image rendered in " << (clock() - t1) / 1000 << " seconds." << endl;
	BMP::savebmp("scene.bmp", imageWidth, imageHeight, dpi, pixels);


	return 0;
}

