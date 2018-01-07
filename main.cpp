#define _CRT_SECURE_NO_WARNINGS

#include <fstream>
#include <cmath>
#include <iostream>
#include <vector>
#include <time.h>
#include <thread>
#include <algorithm>

#include "Vector.h"
#include "Vector2.h"
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
#include "ObjectLoader.h"
#include "Tree.h"


bool TREE = false;
using namespace std;

static const int aadepth = 1;

float clamp(float n, float lower, float upper) {
	return std::max(lower, std::min(n, upper));
}

Camera createCamera(int x, int y, int z) {
	Vector Origin(0, 0, 0);
	Vector X(1, 0, 0);
	Vector Y(0, 1, 0);
	Vector Z(0, 0, 1);
	// model the camera
	Vector campos(x, y, z);
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

bool inShadows(Vector intersection_position, Vector intersection_to_light_direction, vector <Object*> objects, int num, vector <Source*> lights = vector  <Source*>()) {
	return false; //TO DO Shadows are  out for now
	bool shadowed = false;
	Vector actualIntersection = intersection_position;
	if (objects[num]->colour.reflect > 0)
		return false;
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

void diffuse(Colour &diffuse_reflection, Object *&calcObject, double lightAngle, double lightIntensity) {
	diffuse_reflection = calcObject->colour.ColourScalar(lightAngle).ColourScalar(lightIntensity);
	diffuse_reflection.normalizeRGB();
}

void reflection(double lightAngle, vector<Object*> objects, Vector intersection_position, Vector cameraReflect, Colour &reflective_reflection, Light light, Vector lightReflect, Vector intersection_to_camera_direction, int index, int &recursionDepth, Tree &T, double lightIntensity) {
	if ((lightAngle <= 1 && lightAngle >= 0) || objects[index]->colour.getColourReflect() > 0) {
		if (objects[index]->colour.getColourReflect() > 0) {
			Ray reflection_ray(intersection_position, cameraReflect);
			double tempDistance = INFINITY;
			bool reflectionFound = false;
			if (TREE) {
				objects = T.findrelevantObjects(reflection_ray, objects);
			}
			for (Object* reflectedObject : objects) {
				Vector actualIntersection = intersection_position;
				if (reflectedObject->intersect(reflection_ray, actualIntersection) == -1 && !reflectionFound) {
					//if ( lightAngle < 1 && lightAngle > 0) {
					reflective_reflection = light.colour.ColourScalar(pow(lightReflect.dotProduct(intersection_to_camera_direction), 100));		// reflective reflection
					reflective_reflection.normalizeRGB();
					//}
				}
				//Object has reflective property
				else if (reflectedObject != objects[index]) {
					//test if this is the closest object that is then actually reflected
					if (actualIntersection.vectSub(intersection_position).magnitude() < tempDistance && actualIntersection.vectSub(intersection_position).magnitude() > 0.000000001)
					{
						reflectionFound = true;
						tempDistance = actualIntersection.vectSub(intersection_position).magnitude();

						// initialize different lighting values
						Colour diffuse_reflection = Colour(0, 0, 0, 0, 0, 0);
						reflective_reflection = Colour(0, 0, 0, 0, 0, 0);
						Vector cameraPos(intersection_position);
						// calculate lighting and reflection
						Vector intersection_to_light_direction = light.getLightPosition().vectSub(actualIntersection).normalize();		// vector from intersection position to light source	
						Vector intersection_to_camera_direction = cameraPos.vectSub(actualIntersection).normalize();	// vector from intersection to camera position
						Vector normal = reflectedObject->getNormalAt(actualIntersection, intersection_to_light_direction).normalize();		// normal at point of intersection
						double lightAngle = normal.dotProduct(intersection_to_light_direction);												// angle between light ray to the object and the normal of the object	
						double cameraAngle = normal.dotProduct(intersection_to_camera_direction);
						Vector lightReflect = (normal.vectMult(2)).vectMult(lightAngle).vectSub(intersection_to_light_direction);			// reflection vector
						Vector cameraReflect = (normal.vectMult(2)).vectMult(cameraAngle).vectSub(intersection_to_camera_direction);

						Colour c = Colour(0, 0, 0, 0, 0, 0);
						diffuse(diffuse_reflection, reflectedObject, lightAngle, lightIntensity);

						recursionDepth++;
						if (recursionDepth < 20) {
							//reflection on reflection recursion
							ptrdiff_t pos = find(objects.begin(), objects.end(), reflectedObject) - objects.begin();
							reflection(lightAngle, objects, actualIntersection, cameraReflect, c, light, lightReflect, intersection_to_camera_direction, pos, recursionDepth, T, lightIntensity);// calculates reflective reflection
							c = c.ColourScalar(reflectedObject->colour.getColourReflect());
						}
						else {
							c = light.colour.ColourScalar(pow(lightReflect.dotProduct(intersection_to_camera_direction), 100));		// reflective reflection
						}
						reflective_reflection = diffuse_reflection.ColourAdd(c);// reflective reflection
						reflective_reflection.normalizeRGB();
					}
				}
			}
		}
		else {
			reflective_reflection = light.colour.ColourScalar(pow(lightReflect.dotProduct(intersection_to_camera_direction), 50));		// reflective reflection
			reflective_reflection.normalizeRGB();
		}
	}
}

void refraction(Colour &refractive_reflection, Ray &cameraRay, vector<Object*> objects, int index, Vector &intersectionPosition, Object* &hitobject, const float &ior, Vector &intersection_to_light_direction) {
	Vector intersectionPosition2 = intersectionPosition;
	Vector N = objects[index]->getNormalAt(intersectionPosition, intersection_to_light_direction);
	Vector &I = cameraRay.direction;
	float cosi = clamp(-1, 1, I.dotProduct(N));// dotProduct(I, N));
	float etai = 1, etat = ior;
	Vector n = N;
	if (cosi < 0) { cosi = -cosi; }
	else { std::swap(etai, etat); n = N.negative(); }
	float eta = etai / etat;
	float k = 1 - eta * eta * (1 - cosi * cosi);
	Vector my();
	if (k < 0)
		intersectionPosition = intersectionPosition2;
	else {
		Vector direction = I.vectMult(eta).vectAdd(n.vectMult(eta * cosi - sqrtf(k)));
		Ray refractedRay(intersectionPosition, direction);
		for (Object * obj : objects)
		{
			if (obj->intersect(refractedRay, intersectionPosition) == -1) {
				continue;
			}
			else {
				
				if (obj != objects[index])
				{
					hitobject = obj;
					break;
					//return intersectionPosition
				}
			}
		}
	}

}

void fresnel(Object * &calcObject, Vector &intersection_position, Light light, Ray cameraRay, Colour &refractive_reflection, vector<Object*> objects, int i, bool &objectChange, int &pos) {
	if (calcObject->colour.getColourTransparency() > 0) {
		//in case of transparent object, refract ray and calculate new intersection
		Vector intersection_to_light_direction = light.getLightPosition().vectSub(intersection_position).normalize();		// vector from intersection position to light source	
		refraction(refractive_reflection, cameraRay, objects, i, intersection_position, calcObject, 1.3, intersection_to_light_direction);// calculates refractive reflection
		objectChange = true;//needed further down to calculate correct lighting
		pos = find(objects.begin(), objects.end(), calcObject) - objects.begin(); // the object position of the object hit by the refracted ray
	}
}

void renderPart(Tree &T, Camera camera, vector<Object*> objects, int imageHeight, int imageWidth, int widthFrom, int widthTo, vector<Source*> lights, Light light, RGBType*& pixels) {
	// needed global variables
	vector<Object*> objectBackup = objects;
	int thisPixel, aa_index;
	Colour ambient_lighting, diffuse_reflection, reflective_reflection, refractive_reflection, pixel_color;
	double lightIntensity = 1;
	stringstream s;
	//s << widthFrom << " TO " << widthTo << endl;
	std::cout << s.str();
	for (int x = widthFrom; x < widthTo; x++) {
		//std::cout << x << std::endl;
		for (int y = 0; y < imageHeight; y++) {
			thisPixel = y * imageWidth + x;

			double tempRed[aadepth*aadepth];
			double tempGreen[aadepth*aadepth];
			double tempBlue[aadepth*aadepth];

			for (int aax = 0; aax < aadepth; aax++) {
				for (int aay = 0; aay < aadepth; aay++) {
					int recursionDepth = 0;

					aa_index = aay*aadepth + aax;
					tempRed[aa_index] = 128;
					tempGreen[aa_index] = 128;
					tempBlue[aa_index] = 128;

					Vector cam_ray_origin = camera.getCameraPosition();
					//Calc camera ray dependent on perspective
					Vector cam_ray_direction = cameraRay(camera, imageWidth, imageHeight, x, y, aax);
					Ray cameraRay(cam_ray_origin, cam_ray_direction);

					Vector intersection_position;
					double tempDistance = INFINITY;
					if (TREE) {
						vector<Object*>thisItObjects = T.findrelevantObjects(cameraRay, objectBackup);
						objects = thisItObjects;
					}
					for (unsigned int i = 0; i < objects.size(); i++) {
						Object *calcObject = objects[i];
						ptrdiff_t pos = i;
						if (calcObject->intersect(cameraRay, intersection_position) == -1) {
							// ray does not intersect the object
							continue;
						}
						else {
							// ray intersects the object
							//check if object intersection is closest intersection on ray
							if (cameraRay.origin.vectSub(intersection_position).magnitude() < tempDistance)
								tempDistance = cameraRay.origin.vectSub(intersection_position).magnitude();
							else {
								continue;
							}
							// initialize different lighting values
							diffuse_reflection = Colour(0, 0, 0, 0, 0, 0);
							reflective_reflection = Colour(0, 0, 0, 0, 0, 0);
							refractive_reflection = Colour(0, 0, 0, 0, 0, 0);
							Vector oldIntersection = intersection_position;
							bool objectChange = false;
							//FRESNEL
							fresnel(calcObject, intersection_position, light, cameraRay, refractive_reflection, objects, i, objectChange, pos);
							//FRESNEL stop
							// calculate lighting and reflection for current object
							Vector intersection_to_light_direction = light.getLightPosition().vectSub(intersection_position).normalize();		// vector from intersection position to light source	
							Vector intersection_to_camera_direction = camera.getCameraPosition().vectSub(intersection_position).normalize();	// vector from intersection to camera position
							Vector normal = calcObject->getNormalAt(intersection_position, intersection_to_light_direction).normalize();		// normal at point of intersection
							double lightAngle = normal.dotProduct(intersection_to_light_direction);												// angle between light ray to the object and the normal of the object	
							double cameraAngle = normal.dotProduct(intersection_to_camera_direction);
							Vector lightReflect = (normal.vectMult(2)).vectMult(lightAngle).vectSub(intersection_to_light_direction);			// reflection vector
							Vector cameraReflect = (normal.vectMult(2)).vectMult(cameraAngle).vectSub(intersection_to_camera_direction);


							//old object(in case of transparency)
							Vector intersection_to_light_direction2 = light.getLightPosition().vectSub(oldIntersection).normalize();			// vector from intersection position to light source	
							Vector intersection_to_camera_direction2 = camera.getCameraPosition().vectSub(oldIntersection).normalize();			// vector from intersection to camera position
							Vector normal2 = objects[i]->getNormalAt(oldIntersection, intersection_to_light_direction2).normalize();			// normal at point of intersection
							double lightAngle2 = normal2.dotProduct(intersection_to_light_direction2);											// angle between light ray to the object and the normal of the object	
							double cameraAngle2 = normal2.dotProduct(intersection_to_camera_direction2);
							Vector lightReflect2 = (normal2.vectMult(2)).vectMult(lightAngle2).vectSub(intersection_to_light_direction2);		// reflection vector
							Vector cameraReflect2 = (normal2.vectMult(2)).vectMult(cameraAngle2).vectSub(intersection_to_camera_direction2);


							// check for shadows
							//if (!inShadows(intersection_position, intersection_to_light_direction, objects, i, lights)) {
							diffuse(diffuse_reflection, calcObject, lightAngle, lightIntensity);
							if (objectChange) {
								//in case of transparency, still use reflection of original, transparent object, not of surface which is seen through object
								reflection(lightAngle2, objects, oldIntersection, cameraReflect2, reflective_reflection, light, lightReflect2, intersection_to_camera_direction2, i, recursionDepth, T, lightIntensity);// calculates reflective reflection
							}
							else {
								//if not transparent, calculate regular reflection
								reflection(lightAngle, objects, intersection_position, cameraReflect, reflective_reflection, light, lightReflect, intersection_to_camera_direction, pos, recursionDepth, T, lightIntensity);// calculates reflective reflection
							}

							double reflect = calcObject->colour.getColourReflect();
							if (objectChange)
								reflect = objects[i]->colour.getColourReflect();
							if (reflect > 0) {
								Colour a = ambient_lighting.ColourScalar(1 - reflect);
								Colour b = diffuse_reflection.ColourScalar(1 - reflect);
								//reflect = objects[i]->colour.getColourReflect();//always only using original objects colour
								Colour c = reflective_reflection.ColourScalar(reflect);
								pixel_color = a.ColourAdd(b).ColourAdd(c);
							}
							else {
								pixel_color = ambient_lighting.ColourAdd(diffuse_reflection).ColourAdd(reflective_reflection);// .ColourAdd(refractive_reflection);				// addition of reflections and lightings
							}
							//}
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
}

Tree partition(int depth, int maxdepth) {
	Tree T(depth, maxdepth, Vector(-10, -10, -10), Vector(10, 10, 10));
	return T;
}


int main(int argc, char** argv) {
	std::cout << "rendering ..." << std::endl;

	clock_t t1, t2;

	ObjectLoader loader;
	std::vector< Vector > vertices;
	std::vector< Vector2 > uvs;
	std::vector< Vector > normals; // Won't be used at the moment.
	if (TREE) {
		bool res = loader.readFile("bunny.obj", vertices, uvs, normals);
	}

	// images properties
	int imageWidth = 1024;
	int imageHeight = 1024;
	int dpi = 72;
	int numOfPixels = imageHeight * imageWidth;

	Tree T = partition(0, 4);

	Camera camera = createCamera(0, 8, 15);
	// model colors
	Colour color_white(255, 255, 255, 0, 0, 0);
	Colour color_stonegrey(128, 128, 128, 0, 0, 0);
	Colour color_winered(160, 0, 32, 0, 0, 0);
	Colour color_mirror(255, 255, 255, 0, 0.95, 0);
	Colour color_green(34, 139, 34, 0, 0, 0);
	Colour color_blue(50, 80, 200, 0, 0, 0);
	Colour colour_cyan(0, 255, 255, 0, 0, 0);
	Colour mirrorSphere(255, 255, 255, 0, 1, 0);
	Colour darkPurple(57, 0, 130, 0, 0, 0);
	Colour glass(0, 0, 0, 0, 0, 1);

	vector<Object*> objects;
	vector<Object*> triangles;
	// model objects

	for (int i = 0; i < vertices.size(); i += 3) {
		Object * tr = new Triangle(vertices[i], vertices[i + 1], vertices[i + 2], darkPurple);
		triangles.push_back(tr);
		tr->setNormal(normals[i]);
	}

	T.assignTriangles(triangles);

	Triangle ground1(Vector(-8.1, -3.9, 8.5), Vector(8.1, -3.9, 8.5), Vector(8.1, -3.9, -8.5), color_green);
	Triangle ground2(Vector(8.1, -3.9, -8.5), Vector(-8.1, -3.9, -8.5), Vector(-8.1, -3.9, 8.5), color_green);
	Triangle ceiling1(Vector(-8.1, 7.9, 8.5), Vector(8.1, 7.9, 8.5), Vector(8.1, 7.9, -8.5), color_stonegrey);
	Triangle ceiling2(Vector(8.1, 7.9, -8.5), Vector(-8.1, 7.9, -8.5), Vector(-8.1, 7.9, 8.5), color_stonegrey);
	Triangle wallBack1(Vector(-8.5, -4.1, -8), Vector(8.5, 8.1, -8), Vector(-8.5, 8.1, -8), color_stonegrey);
	Triangle wallBack2(Vector(8.5, 8.1, -8), Vector(8.5, -4.1, -8), Vector(-8.5, -4.1, -8), color_stonegrey);
	Triangle wallFront1(Vector(-8.1 * 30, -4.01 * 30, 8.1 * 30), Vector(8.1 * 30, 8.1 * 30, 8.1 * 30), Vector(-8.1 * 30, 8.1 * 30, 8.1 * 30), color_stonegrey);
	Triangle wallFront2(Vector(8.1 * 30, 8.1 * 30, 8.1 * 30), Vector(8.1 * 30, -4.01 * 30, 8.1 * 30), Vector(-8.1 * 30, -4.01 * 30, 8.1 * 30), color_stonegrey);
	Triangle wallRight1(Vector(-8, 8.1, -8.1), Vector(-8, -4.1, -8.1), Vector(-8, 8.1, 8.1), color_blue);
	Triangle wallRight2(Vector(-8, 8.1, 8.1), Vector(-8, -4.1, -8.1), Vector(-8, -4.1, 8.1), color_blue);
	Triangle wallLeft1(Vector(8, 8.1, -8.1), Vector(8, -4.1, -8.1), Vector(8, 8.1, 8.1), color_winered);
	Triangle wallLeft2(Vector(8, 8.1, 8.1), Vector(8, -4.1, -8.1), Vector(8, -4.1, 8.1), color_winered);
	Sphere sphere(Vector(3, -2, 1), 0.8, color_winered);
	Sphere sphere2(Vector(0, 1, 0), 3, mirrorSphere);
	Sphere sphere3(Vector(0, 6, 0), 0.2, color_green);
	Sphere sphere4(Vector(7, 1, -10), 2, color_winered);

	objects.push_back(&ground1);
	objects.push_back(&ground2);
	objects.push_back(&wallLeft1);
	objects.push_back(&wallLeft2);
	objects.push_back(&sphere);
	objects.push_back(&sphere2);
	objects.push_back(&sphere3);
	objects.push_back(&wallFront1);
	objects.push_back(&wallFront2);
	objects.push_back(&wallRight1);
	objects.push_back(&wallRight2);
	objects.push_back(&ceiling1);
	objects.push_back(&ceiling2);
	objects.push_back(&wallBack1);
	objects.push_back(&wallBack2);
	//objects.push_back(&sphere4);

	// model light sources
	Light light(Vector(0, 7, 0), color_white);
	vector<Source*> lights;
	lights.push_back(&light);
	t1 = clock();
	const int numThreads = 8;
	std::vector<std::thread> threads;
	RGBType *pixels = new RGBType[numOfPixels];
	for (int i = 0; i < numThreads; ++i) {
		threads.push_back(std::thread(renderPart, T, camera, objects, imageHeight, imageWidth, i* (imageWidth / numThreads), (i + 1) * (imageWidth / numThreads), lights, light, std::ref(pixels)));
	}
	for (auto& t : threads) {
		t.join();
	}
	BMP::savebmp("scene.bmp", imageWidth, imageHeight, dpi, pixels);

	t2 = clock();
	float diff = ((float)t2 - (float)t1) / 1000;
	cout << "rendered " << imageWidth << " x " << imageHeight << " image in " << diff << " seconds, using AA depth of " << aadepth << endl;
	return 0;
}

