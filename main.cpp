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

static const int aadepth = 1;

bool inShadows(Vector intersection_position, Vector intersection_to_light_direction, vector <Object*> objects, int num, vector <Source*> lights = vector  <Source*>()) {
	return false;// TO DO Shadows are  out for now
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

void reflection(double lightAngle, vector<Object*> objects, Vector intersection_position, Vector cameraReflect, Colour &reflective_reflection, Light light, Vector lightReflect, Vector intersection_to_camera_direction, int index, int &recursionDepth) {
	if ((lightAngle < 1 && lightAngle > 0) || objects[index]->colour.getColourReflect() > 0) {
		if (objects[index]->colour.getColourReflect() > 0) {
			Ray reflection_ray(intersection_position, cameraReflect);
			double tempDistance = INFINITY;
			bool reflectionFound = false;
			for (Object* reflectedObject : objects) {
				Vector actualIntersection = intersection_position;
				if (reflectedObject->intersect(reflection_ray, actualIntersection) == -1 && !reflectionFound) {
					if (lightAngle < 1 && lightAngle > 0) {
						reflective_reflection = light.colour.ColourScalar(pow(lightReflect.dotProduct(intersection_to_camera_direction), 100));		// reflective reflection
						reflective_reflection.normalizeRGB();
					}
				}
				else if (reflectedObject != objects[index]) {//Object has reflective property
					//test if this is the closest object that is then actually reflected
					if (actualIntersection.vectSub(intersection_position).magnitude() < tempDistance && actualIntersection.vectSub(intersection_position).magnitude() > 0.01)
					{
						double lightIntensity = 0.7;
						double material = 1;

						reflectionFound = true;
						tempDistance = actualIntersection.vectSub(intersection_position).magnitude();

						// initialize different lighting values
						Colour diffuse_reflection = Colour(0, 0, 0, 0, 0);
						reflective_reflection = Colour(0, 0, 0, 0, 0);
						Vector cameraPos(intersection_position);
						// calculate lighting and reflection
						Vector intersection_to_light_direction = light.getLightPosition().vectSub(actualIntersection).normalize();		// vector from intersection position to light source	
						Vector intersection_to_camera_direction = cameraPos.vectSub(actualIntersection).normalize();	// vector from intersection to camera position
						Vector normal = reflectedObject->getNormalAt(actualIntersection, intersection_to_light_direction).normalize();		// normal at point of intersection
						double lightAngle = normal.dotProduct(intersection_to_light_direction);												// angle between light ray to the object and the normal of the object	
						double cameraAngle = normal.dotProduct(intersection_to_camera_direction);
						Vector lightReflect = (normal.vectMult(2)).vectMult(lightAngle).vectSub(intersection_to_light_direction);			// reflection vector
						Vector cameraReflect = (normal.vectMult(2)).vectMult(cameraAngle).vectSub(intersection_to_camera_direction);

						//// check for shadows
						//if (!inShadows(actualIntersection, intersection_to_light_direction, objects, i)) {

						//	reflection(lightAngle, objects, intersection_position, cameraReflect, reflective_reflection, light, lightReflect, intersection_to_camera_direction, i);// calculates reflective reflection
						//	double reflect = objects[i]->colour.reflect;
						//	double reflectInvert = 1 - reflect;
						//	if (objects[i]->colour.getColourReflect() > 0) {
						//Colour a = ambient_lighting.ColourScalar(1 - reflectedObject->colour.getColourReflect());
						Colour c = Colour(0, 0, 0, 0, 0);
						Colour b = Colour(0, 0, 0, 0, 0);
						diffuse_reflection = reflectedObject->colour.ColourScalar(lightAngle).ColourScalar(lightIntensity).ColourScalar(material).ColourScalar(1 - reflectedObject->colour.getColourReflect());	// diffuse reflection
						diffuse_reflection.normalizeRGB();
						b = diffuse_reflection;
						recursionDepth++;
						if (recursionDepth < 10) {
							//reflection on reflection recursion
							ptrdiff_t pos = find(objects.begin(), objects.end(), reflectedObject) - objects.begin();
							reflection(lightAngle, objects, actualIntersection, cameraReflect, c, light, lightReflect, intersection_to_camera_direction, pos, recursionDepth);// calculates reflective reflection
						}
						else {
							c = light.colour.ColourScalar(pow(lightReflect.dotProduct(intersection_to_camera_direction), 100));		// reflective reflection
						}
						reflective_reflection = b.ColourAdd(c);// reflective reflection
						reflective_reflection.normalizeRGB();
					}
				}
			}
		}
		else {
			reflective_reflection = light.colour.ColourScalar(pow(lightReflect.dotProduct(intersection_to_camera_direction), 100));		// reflective reflection
			reflective_reflection.normalizeRGB();
		}
	}
}

Camera createCamera() {
	Vector Origin(0, 0, 0);
	Vector X(1, 0, 0);
	Vector Y(0, 1, 0);
	Vector Z(0, 0, 1);
	// model the camera
	Vector campos(0, 8, 15);
	Vector look_at(Origin);
	Vector diff_btw(campos.x - look_at.x, campos.y - look_at.y, campos.z - look_at.z);
	Vector camdir = diff_btw.negative().normalize();
	Vector camright = Y.crossProduct(camdir).normalize();
	Vector camdown = camright.crossProduct(camdir);
	Camera camera(campos, camdir, camright, camdown);
	return camera;
}

Vector cameraRay(Camera camera, int imageWidth, int imageHeight, int x, int y, int aax) {
	double perspectiveX, perspectiveY;
	double aspectratio = (double)imageWidth / (double)imageHeight;
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
	else {	// using anti aliasing
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
	return camera.getCameraDirection().vectAdd(camera.getCameraRight().vectMult(perspectiveX - 0.5).vectAdd(camera.getCameraDown().vectMult(perspectiveY - 0.5))).normalize();

}

int main(int argc, char** argv) {
	std::cout << "rendering ..." << std::endl;

	clock_t t1, t2;

	// images properties
	int imageWidth = 1024;
	int imageHeight = 856;
	int dpi = 72;
	int numOfPixels = imageHeight * imageWidth;

	Camera camera = createCamera();
	// model colors
	Colour color_white(255, 255, 255, 0, 0);
	Colour color_stonegrey(128, 128, 128, 0, 0);
	Colour color_winered(160, 0, 32, 0, 0);
	Colour color_mirror(255, 255, 255, 0, 1);
	Colour color_green(34, 139, 34, 0, 0);
	Colour color_blue(50, 80, 200, 0, 0);
	Colour colour_cyan(0, 255, 255, 0, 0);
	Colour mirrorSphere(255, 255, 255, 0, 1);
	Colour darkPurple(57, 0, 130, 0, 0);
	Colour glass(57, 0, 130, 0, 0);

	vector<Object*> objects;
	// model objects
	Triangle ground1(Vector(-8, -4, 8), Vector(8, -4, 8), Vector(8, -4, -8), color_mirror);
	Triangle ground2(Vector(8, -4, -8), Vector(-8, -4, -8), Vector(-8, -4, 8), color_mirror);
	Triangle ceiling1(Vector(-8, 8, 8), Vector(8, 8, 8), Vector(8, 8, -8), darkPurple);
	Triangle ceiling2(Vector(8, 8, -8), Vector(-8, 8, -8), Vector(-8, 8, 8), darkPurple);
	Triangle wallBack1(Vector(-8, -4, -8), Vector(8, 8, -8), Vector(-8, 8, -8), color_stonegrey);
	Triangle wallBack2(Vector(8, 8, -8), Vector(8, -4, -8), Vector(-8, -4, -8), color_stonegrey);
	Triangle wallRight1(Vector(-8, 8, -8), Vector(-8, -4, -8), Vector(-8, 8, 8), color_blue);
	Triangle wallRight2(Vector(-8, 8, 8), Vector(-8, -4, -8), Vector(-8, -4, 8), color_blue);
	Triangle wallLeft1(Vector(8, 8, -8), Vector(8, -4, -8), Vector(8, 8, 8), color_winered);
	Triangle wallLeft2(Vector(8, 8, 8), Vector(8, -4, -8), Vector(8, -4, 8), color_winered);
	Sphere sphere(Vector(3, -2, 1), 0.8, colour_cyan);
	Sphere sphere2(Vector(0, 4, 0), 3, mirrorSphere);
	Sphere sphere3(Vector(-2, 1, 3), 0.8, color_green);

	objects.push_back(&wallBack1);
	objects.push_back(&wallBack2);
	objects.push_back(&ground1);
	objects.push_back(&ground2);
	objects.push_back(&sphere);
	objects.push_back(&sphere2);
	objects.push_back(&sphere3);
	objects.push_back(&wallLeft1);
	objects.push_back(&wallLeft2);
	objects.push_back(&wallRight1);
	objects.push_back(&wallRight2);
	objects.push_back(&ceiling1);
	objects.push_back(&ceiling2);

	// model light sources
	Light light(Vector(0, 10, 0), color_white);
	vector<Source*> lights;
	lights.push_back(&light);

	// needed global variables
	int thisPixel, aa_index;
	RGBType *pixels = new RGBType[numOfPixels];
	Colour ambient_lighting, diffuse_reflection, reflective_reflection, pixel_color;
	double lightIntensity = 0.7;
	double material = 1;
	float temp[10];

	for (int testing = 0; testing < 1; testing++) {			// loop for testing rendering time
		t1 = clock();
		for (int x = 0; x < imageWidth; x++) {
			std::cout << x << std::endl;
			for (int y = 0; y < imageHeight; y++) {
				thisPixel = y * imageWidth + x;

				double tempRed[aadepth*aadepth];
				double tempGreen[aadepth*aadepth];
				double tempBlue[aadepth*aadepth];

				for (int aax = 0; aax < aadepth; aax++) {
					for (int aay = 0; aay < aadepth; aay++) {
						int recursionDepth = 0;

						srand((uint32_t)time(0));

						aa_index = aay*aadepth + aax;
						tempRed[aa_index] = 128;
						tempGreen[aa_index] = 128;
						tempBlue[aa_index] = 128;

						Vector cam_ray_origin = camera.getCameraPosition();
						//Calc camera ray dependent on perspective
						Vector cam_ray_direction = cameraRay(camera, imageWidth, imageHeight, x, y, aax);
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
								diffuse_reflection = Colour(0, 0, 0, 0, 0);
								reflective_reflection = Colour(0, 0, 0, 0, 0);
								ambient_lighting = objects[i]->colour.ColourScalar(0.1);															// ambient lighting

								// calculate lighting and reflection
								Vector intersection_to_light_direction = light.getLightPosition().vectSub(intersection_position).normalize();		// vector from intersection position to light source	
								Vector intersection_to_camera_direction = camera.getCameraPosition().vectSub(intersection_position).normalize();	// vector from intersection to camera position
								Vector normal = objects[i]->getNormalAt(intersection_position, intersection_to_light_direction).normalize();		// normal at point of intersection
								double lightAngle = normal.dotProduct(intersection_to_light_direction);												// angle between light ray to the object and the normal of the object	
								double cameraAngle = normal.dotProduct(intersection_to_camera_direction);
								Vector lightReflect = (normal.vectMult(2)).vectMult(lightAngle).vectSub(intersection_to_light_direction);			// reflection vector
								Vector cameraReflect = (normal.vectMult(2)).vectMult(cameraAngle).vectSub(intersection_to_camera_direction);

								// check for shadows
								if (!inShadows(intersection_position, intersection_to_light_direction, objects, i, lights)) {
									diffuse_reflection = objects[i]->colour.ColourScalar(lightAngle).ColourScalar(lightIntensity).ColourScalar(material);	// diffuse reflection
									diffuse_reflection.normalizeRGB();
									reflection(lightAngle, objects, intersection_position, cameraReflect, reflective_reflection, light, lightReflect, intersection_to_camera_direction, i, recursionDepth);// calculates reflective reflection
									double reflect = objects[i]->colour.reflect;
									double reflectInvert = 1 - reflect;
									if (objects[i]->colour.getColourReflect() > 0) {
										Colour a = ambient_lighting.ColourScalar(reflectInvert);
										Colour b = diffuse_reflection.ColourScalar(reflectInvert);
										Colour c = reflective_reflection.ColourScalar(reflect);
										pixel_color = a.ColourAdd(b).ColourAdd(c);
									}
									else {
										pixel_color = ambient_lighting.ColourAdd(diffuse_reflection).ColourAdd(reflective_reflection);				// addition of reflections and lightings
									}
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

						pixels[thisPixel].r = (int)avgRed;
						pixels[thisPixel].g = (int)avgGreen;
						pixels[thisPixel].b = (int)avgBlue;
					}
				}
			}
		}

		BMP::savebmp("scene.bmp", imageWidth, imageHeight, dpi, pixels);

		t2 = clock();
		float diff = ((float)t2 - (float)t1) / 1000;

		temp[testing] = diff;

		cout << "rendered " << imageWidth << " x " << imageHeight << " image in " << diff << " seconds, using AA depth of " << aadepth << endl;

	}

	double average = 0;

	for (int i = 0; i < 10; i++) {
		average += temp[i];
	}

	average /= 10;

	cout << "average rendering time: " << average << " seconds" << endl;

	//system("Pause");

	return 0;
}

