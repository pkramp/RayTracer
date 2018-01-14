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
bool BUNNY = false;
bool DEBUG = false;
using namespace std;

static const int aadepth = 1;

float clamp(float n, float lower, double upper) {
	return std::max(lower, std::min(n, (float)upper));
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

void closestIntersection(Ray ray, Vector &intersectionPosition, vector<Object*> objects, Tree T, Object * obj, Object* &hitobject, bool sameallowed) {
	double tempDistance = INFINITY;
	if (TREE)
		objects = T.findrelevantObjects(ray, objects);
	Vector tempIntersection = intersectionPosition;
	Vector intersectionBackup = intersectionPosition;
	bool hitself = false;
	for (Object* object : objects) {
		ray.origin = ray.origin.vectAdd(ray.direction.vectMult(0.0000001));
		if ((object == obj && !sameallowed) || object->intersect(ray, intersectionBackup) == -1) {
			// ray does not intersect the object
			continue;
		}
		else {
			tempIntersection = intersectionBackup;
			intersectionBackup = intersectionPosition;
			// ray intersects the object
			//check if object intersection is closest intersection on ray
			if (ray.origin.vectSub(tempIntersection).magnitude() < tempDistance && ray.origin.vectSub(tempIntersection).magnitude() > 0.000000001) {
				if (object == obj)
					hitself = true;
				else {
					hitself = false;
				}
				tempDistance = ray.origin.vectSub(tempIntersection).magnitude();
				hitobject = object;
				intersectionPosition = tempIntersection;
			}
			else {
				continue;
			}
		}
	}
	if (DEBUG && hitself)
		cout << "hits self from" << ray.origin.print() << " To " << intersectionPosition.print() << "With direction: " << ray.direction.print() << endl;
}

bool inShadows(Vector intersection_position, Vector intersection_to_light_direction, vector <Object*> objects, int num, Light light, Tree T) {
	//return false; //TO DO Shadows are  out for now
	bool shadowed = false;
	Vector actualIntersection = intersection_position;
	if (objects[num]->colour.reflect > 0 || objects[num]->colour.transparency > 0)
		return false;

	Object * searched = nullptr;
	Ray shadow(intersection_position, intersection_to_light_direction);
	if (TREE)
		objects = T.findrelevantObjects(shadow, objects);
	closestIntersection(shadow, actualIntersection, objects, T, objects[num], searched, false);//finds closest intersection to reflection_ray
	if (light.getLightPosition().vectSub(actualIntersection).magnitude() > intersection_position.vectSub(actualIntersection).magnitude()) {
		shadowed = true;
	}
	return shadowed;
}

void diffuse(Colour &diffuse_reflection, Object *&calcObject, Light &light, Vector &intersectionPosition, double lightIntensity, vector<Object*> objects, Tree T) {
	Vector intersection_to_light_direction = light.getLightPosition().vectSub(intersectionPosition).normalize();		// vector from intersection position to light source	
	Vector normal = calcObject->getNormalAt(intersectionPosition, intersection_to_light_direction).normalize();			// normal at point of intersection
	double lightAngle = normal.dotProduct(intersection_to_light_direction);
	ptrdiff_t index = find(objects.begin(), objects.end(), calcObject) - objects.begin();							// angle between light ray to the object and the normal of the object	
	if (!inShadows(intersectionPosition, intersection_to_light_direction, objects, index, light, T)) {
		diffuse_reflection = calcObject->colour.ColourScalar(lightAngle).ColourScalar(lightIntensity);
		diffuse_reflection.normalizeRGB();
	}
}

void reflection(double lightAngle, vector<Object*> objects, Vector intersection_position, Vector cameraReflect, Colour &reflective_reflection, Light light, Vector lightReflect, Vector intersection_to_camera_direction, int index, int &recursionDepth, Tree &T, double lightIntensity) {
	if ((lightAngle < 1 && lightAngle > 0) || objects[index]->colour.getColourReflect() > 0) {
		if (objects[index]->colour.getColourReflect() > 0) {
			Ray reflection_ray(intersection_position, cameraReflect);
			if (TREE) {
				objects = T.findrelevantObjects(reflection_ray, objects);
			}
			Object* reflectedObject = nullptr;
			Vector actualIntersection = intersection_position;
			closestIntersection(reflection_ray, actualIntersection, objects, T, objects[index], reflectedObject, false);//finds closest intersection to reflection_ray
			if (reflectedObject != nullptr) {
				// initialize different lighting values
				Colour diffuse_reflection = Colour(0, 0, 0, 0, 0, 0);
				reflective_reflection = Colour(0, 0, 0, 0, 0, 0);
				// calculate lighting and reflection
				Vector intersection_to_light_direction = (light.getLightPosition().vectSub(actualIntersection)).normalize();			// vector from intersection position to light source	
				Vector intersection_to_camera_direction = (intersection_position.vectSub(actualIntersection)).normalize();			// vector from intersection to camera position
				Vector normal = (reflectedObject->getNormalAt(actualIntersection, intersection_to_light_direction)).normalize();		// normal at point of intersection
				double lightAngle = normal.dotProduct(intersection_to_light_direction);												// angle between light ray to the object and the normal of the object	
				double cameraAngle = normal.dotProduct(intersection_to_camera_direction);
				Vector lightReflect = ((normal.vectMult(2)).vectMult(lightAngle).vectSub(intersection_to_light_direction)).normalize();			// reflection vector
				Vector cameraReflect = ((normal.vectMult(2)).vectMult(cameraAngle).vectSub(intersection_to_camera_direction)).normalize();

				Colour reflection_colour = Colour(0, 0, 0, 0, 0, 0);
				diffuse(diffuse_reflection, reflectedObject, light, actualIntersection, lightIntensity, objects, T);
				diffuse_reflection = diffuse_reflection.ColourScalar(1 - reflectedObject->colour.reflect);
				if (recursionDepth < 20 && reflectedObject->colour.reflect >0/* && lightAngle < 0*/) {
					//reflection on reflection recursion
					recursionDepth++;
					ptrdiff_t pos = find(objects.begin(), objects.end(), reflectedObject) - objects.begin();
					reflection(lightAngle, objects, actualIntersection, cameraReflect, reflection_colour, light, lightReflect, intersection_to_camera_direction, pos, recursionDepth, T, lightIntensity);// calculates reflective reflection
					reflection_colour = reflection_colour.ColourScalar(reflectedObject->colour.getColourReflect());
				}
				else {
					if (lightAngle < 1 && lightAngle > 0)
						reflection_colour = light.colour.ColourScalar(pow(lightReflect.dotProduct(intersection_to_camera_direction), 400));		// reflective reflection
				}
				reflective_reflection = diffuse_reflection.ColourAdd(reflection_colour);// reflective reflection
				reflective_reflection.normalizeRGB();
			}
		}
		else {
			reflective_reflection = light.colour.ColourScalar(pow(lightReflect.dotProduct(intersection_to_camera_direction), 400));		// reflective reflection
			reflective_reflection.normalizeRGB();
		}
	}
}

void refraction(Colour &refractive_reflection, Ray &cameraRay, vector<Object*> objects, int index, Vector &intersectionPosition, Object* &hitobject, const float &ior, Vector &intersection_to_light_direction, Tree T, int &recursionDepth, Light light) {
	int ownDepth = recursionDepth;
	Vector intersectionPosition2 = intersectionPosition;
	Vector N = objects[index]->getNormalAt(intersectionPosition, intersection_to_light_direction);
	Vector &I = cameraRay.direction.normalize();
	float cosi = clamp(-1, 1, I.dotProduct(N));
	float etai = 1, etat = ior;
	Vector n = N;
	if (cosi < 0) { cosi = -cosi; }
	else { std::swap(etai, etat); n = N.negative(); }
	float eta = etai / etat;
	float k = 1 - eta * eta * (1 - cosi * cosi);
	if (k < 0)
		intersectionPosition = Vector(10, 10, 10);// = intersectionPosition2;
	else {
		Vector direction = I.vectMult(eta).vectAdd(n.vectMult(eta * cosi - sqrtf(k)));
		direction = direction.normalize();
		Ray refractedRay(intersectionPosition, direction);
		if (ownDepth>0)
			closestIntersection(refractedRay, intersectionPosition2, objects, T, objects[index], hitobject, false);
		else
			closestIntersection(refractedRay, intersectionPosition2, objects, T, objects[index], hitobject, true);
		intersectionPosition = intersectionPosition2;
		if (hitobject->colour.transparency > 0 && recursionDepth < 5) {
			Vector intersectionPosition3 = intersectionPosition2;
			Vector intersection_to_light_direction2 = (light.getLightPosition().vectSub(intersectionPosition3)).normalize();		// vector from intersection position to light source	
			recursionDepth++;
			refraction(refractive_reflection, refractedRay, objects, index, intersectionPosition3, hitobject, ior, intersection_to_light_direction2, T, recursionDepth, light);
			intersectionPosition = intersectionPosition3;
		}
		else {
			intersectionPosition = intersectionPosition2;
		}
		if (DEBUG && ownDepth == 0) {
			cout << "Refracted Ray hits " << hitobject->colour.red << " " << hitobject->colour.green << " " << hitobject->colour.blue << endl;
			cout << "From Object: " << objects[index]->colour.red << endl;
		}
	}
}

void fresnel(Vector &I, Vector &N, const float &ior, float &kr)
{
	float cosi = clamp(I.dotProduct(N), -1, 1);
	float etai = 1, etat = ior;
	if (cosi > 0) { std::swap(etai, etat); }
	// Compute sini using Snell's law
	float sint = etai / etat * sqrtf(std::max(0.f, 1 - cosi * cosi));
	// Total internal reflection
	if (sint >= 1) {
		kr = 1;
		cout << "Internal" << endl;
	}
	else {
		float cost = sqrtf(std::max(0.f, 1 - sint * sint));
		cosi = fabsf(cosi);
		float Rs = ((etat * cosi) - (etai * cost)) / ((etat * cosi) + (etai * cost));
		float Rp = ((etai * cosi) - (etat * cost)) / ((etai * cosi) + (etat * cost));
		kr = (Rs * Rs + Rp * Rp) / 2;
	}
	// As a consequence of the conservation of energy, transmittance is given by:
	// kt = 1 - kr;
}

void refractionPosition(Object * &calcObject, Vector &intersection_position, Light light, Ray cameraRay, Colour & refractiveReflection, vector<Object*> objects, int i, bool &transparency, Camera &camera, Tree T, int recursionDepth, double lightIntensity, float &kr) {
	//in case of transparent object, refract ray and calculate new intersection
	Vector intersection_to_light_direction = light.getLightPosition().vectSub(intersection_position);		// vector from intersection position to light source	
	intersection_to_light_direction = intersection_to_light_direction.normalize();
	fresnel(cameraRay.direction.normalize(), calcObject->getNormalAt(intersection_position, intersection_to_light_direction), 1.5, kr);
	int recursiondepth = 0;
	refraction(refractiveReflection, cameraRay, objects, i, intersection_position, calcObject, 1.5, intersection_to_light_direction, T, recursiondepth, light);// calculates refractive reflection
	refractiveReflection = refractiveReflection.ColourScalar(1 - kr);
	transparency = true;//needed further down to calculate correct lighting
}


void allReflections(Object * &calcObject, Vector &intersection_position, Light light, Ray cameraRay, Colour & refractiveReflection, Colour &reflective_reflection, vector<Object*> objects, int i, bool &transparency, Camera &camera, Tree T, int recursionDepth, double lightIntensity, float &kr) {
	Vector oldIntersection = intersection_position;
	kr = 1;
	if (calcObject->colour.getColourTransparency() > 0) {
		refractionPosition(calcObject, intersection_position, light, cameraRay, refractiveReflection, objects, i, transparency, camera, T, recursionDepth, lightIntensity, kr);
	}

	// calculate lighting and reflection for current object
	Vector intersection_to_light_direction = (light.getLightPosition().vectSub(intersection_position)).normalize();		// vector from intersection position to light source	
	Vector intersection_to_camera_direction = (camera.getCameraPosition().vectSub(intersection_position)).normalize();	// vector from intersection to camera position
	Vector normal = (calcObject->getNormalAt(intersection_position, intersection_to_light_direction)).normalize();		// normal at point of intersection
	double lightAngle = normal.dotProduct(intersection_to_light_direction);												// angle between light ray to the object and the normal of the object	
	double cameraAngle = normal.dotProduct(intersection_to_camera_direction);
	Vector lightReflect = (normal.vectMult(2).vectMult(lightAngle).vectSub(intersection_to_light_direction)).normalize();			// reflection vector
	Vector cameraReflect = (normal.vectMult(2).vectMult(cameraAngle).vectSub(intersection_to_camera_direction)).normalize();


	//old object(in case of transparency)
	Vector intersection_to_light_direction2 = (light.getLightPosition().vectSub(oldIntersection)).normalize();			// vector from intersection position to light source	
	Vector intersection_to_camera_direction2 = (camera.getCameraPosition().vectSub(oldIntersection)).normalize();			// vector from intersection to camera position
	Vector normal2 = (objects[i]->getNormalAt(oldIntersection, intersection_to_light_direction2)).normalize();			// normal at point of intersection
	double lightAngle2 = normal2.dotProduct(intersection_to_light_direction2);											// angle between light ray to the object and the normal of the object	
	double cameraAngle2 = normal2.dotProduct(intersection_to_camera_direction2);
	Vector lightReflect2 = (normal2.vectMult(2)).vectMult(lightAngle2).vectSub(intersection_to_light_direction2).normalize();		// reflection vector
	Vector cameraReflect2 = (normal2.vectMult(2)).vectMult(cameraAngle2).vectSub(intersection_to_camera_direction2).normalize();


	// check for shadows
	//if (!inShadows(intersection_position, intersection_to_light_direction, objects, i, lights)) {
	if (transparency) {
		//in case of transparency, still use reflection of original, transparent object, not of surface which is seen through object
		reflection(lightAngle2, objects, oldIntersection, cameraReflect2, reflective_reflection, light, lightReflect2, intersection_to_camera_direction2, i, recursionDepth, T, lightIntensity);// calculates reflective reflection
		reflective_reflection = reflective_reflection.ColourScalar(kr);
	}
	else {
		//if not transparent, calculate regular reflection
		reflection(lightAngle, objects, intersection_position, cameraReflect, reflective_reflection, light, lightReflect, intersection_to_camera_direction, i, recursionDepth, T, lightIntensity);// calculates reflective reflection
	}
}

void renderPart(Tree &T, Camera camera, vector<Object*> objects, int imageHeight, int imageWidth, int widthFrom, int widthTo, vector<Source*> lights, Light light, RGBType*& pixels) {
	// needed global variables
	vector<Object*> objectBackup = objects;
	int thisPixel, aa_index;
	Colour ambient_lighting, diffuse_reflection, reflective_reflection, refractive_reflection, pixel_color;
	double lightIntensity = 1;
	stringstream s;
	s << widthFrom << " TO " << widthTo << endl;
	std::cout << s.str();
	for (int x = widthFrom; x < widthTo; x++) {
		std::cout << x << std::endl;
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
					Ray cameraRay(cam_ray_origin, cam_ray_direction.normalize());

					Vector intersection_position;
					double tempDistance = INFINITY;
					if (TREE) {
						vector<Object*>thisItObjects = T.findrelevantObjects(cameraRay, objectBackup);
						objects = thisItObjects;
					}
					for (unsigned int i = 0; i < objects.size(); i++) {
						Object *calcObject = objects[i];
						if (calcObject->intersect(cameraRay, intersection_position) == -1) {
							// ray does not intersect the object
							continue;
						}
						else {
							//cout << "hits self from" << cameraRay.origin.print() << " To " << intersection_position.print() << "With direction: " << cameraRay.direction.print() << endl;
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
							bool transparency = false;
							float kr;
							Vector oldIntersection = intersection_position;
							//FRESNEL
							allReflections(calcObject, intersection_position, light, cameraRay, refractive_reflection, reflective_reflection, objects, i, transparency, camera, T, recursionDepth, lightIntensity, kr);
							//FRESNEL stop
							diffuse(diffuse_reflection, objects[i], light, oldIntersection, lightIntensity, objects, T);//diffuse reflection of normal object
							diffuse(refractive_reflection, calcObject, light, intersection_position, lightIntensity, objects, T);//diffuse reflection of refracted ray
							if (transparency)//add these two diffuse reflections 
								diffuse_reflection = (refractive_reflection.ColourScalar(1 - kr).ColourScalar(objects[i]->colour.transparency)).ColourAdd(diffuse_reflection.ColourScalar(1 - objects[i]->colour.transparency));
							Vector intersection_to_light_direction = light.getLightPosition().vectSub(intersection_position).normalize();		// vector from intersection position to light source	
							double reflect = calcObject->colour.getColourReflect();
							if (transparency)
								reflect = objects[i]->colour.getColourReflect();
							if (reflect > 0) {
								Colour a = ambient_lighting.ColourScalar(1 - reflect);
								Colour b;
								if (transparency)
									b = diffuse_reflection;
								else
									b = diffuse_reflection.ColourScalar(1 - reflect);
								reflective_reflection.normalizeRGB();
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
	if (TREE && BUNNY) {
		bool res = loader.readFile("bunny.obj", vertices, uvs, normals);
	}

	Tree T = partition(0, 4);

	double refl = 0;
	// model colors
	Colour color_white(255, 255, 255, 0, 0, 0);
	Colour color_yellow(255, 255, 0, 0, 0, 0);
	Colour color_stonegrey(128, 128, 128, 0, refl, 0);
	Colour color_winered(160, 0, 32, 0, 0, 0);
	Colour color_winered2(160, 0, 32, 0, 1, 0);
	Colour color_mirror(255, 255, 255, 0, 0.95, 0);
	Colour color_green(34, 139, 34, 0, 1, 0);
	Colour color_blue(50, 80, 200, 0, refl, 0);
	Colour color_cyan(0, 255, 255, 0, 0, 0);
	Colour color_perfectMirror(255, 255, 255, 0, 1, 0);
	Colour color_darkPurple(57, 0, 130, 0, refl, 0);
	Colour color_glass(0, 0, 0, 0, 1, 1);

	vector<Object*> objects;
	vector<Object*> triangles;
	// model objects

	for (int i = 0; i < vertices.size(); i += 3) {
		Object * tr = new Triangle(vertices[i], vertices[i + 1], vertices[i + 2], color_white);
		triangles.push_back(tr);
		//tr->setNormal(normals[i]);
	}

	T.assignTriangles(triangles);

	Triangle ground1(Vector(-8.1, -3.9, 2 * 8.5), Vector(8.1, -3.9, 2*8.5), Vector(8.1, -3.9, -8.5), color_green);
	Triangle ground2(Vector(8.1, -3.9, -8.5), Vector(-8.1, -3.9, -8.5), Vector(-8.1, -3.9, 2 * 8.5), color_green);
	Triangle ceiling1(Vector(-8.1, 7.9, 8.5), Vector(8.1, 7.9, 8.5), Vector(8.1, 7.9, -8.5), color_white);
	Triangle ceiling2(Vector(8.1, 7.9, -8.5), Vector(-8.1, 7.9, -8.5), Vector(-8.1, 7.9, 8.5), color_white);
	Triangle wallBack1(Vector(-8.5, -4.1, -8), Vector(8.5, 8.1, -8), Vector(-8.5, 8.1, -8), color_stonegrey);
	Triangle wallBack2(Vector(8.5, 8.1, -8), Vector(8.5, -4.1, -8), Vector(-8.5, -4.1, -8), color_stonegrey);
	Triangle wallFront1(Vector(-8.1 * 300, -4.01 * 300, 8.1 * 3), Vector(8.1 * 300, 8.1 * 300, 8.1 * 3), Vector(-8.1 * 300, 8.1 * 300, 8.1 * 3), color_darkPurple);
	Triangle wallFront2(Vector(8.1 * 300, 8.1 * 300, 8.1 * 3), Vector(8.1 * 300, -4.01 * 300, 8.1 * 3), Vector(-8.1 * 300, -4.01 * 300, 8.1 * 3), color_darkPurple);
	Triangle wallRight1(Vector(-8, 8.1, -8.1), Vector(-8, -4.1, -8.1), Vector(-8, 8.1, 8.1), color_blue);
	Triangle wallRight2(Vector(-8, 8.1, 8.1), Vector(-8, -4.1, -8.1), Vector(-8, -4.1, 8.1), color_blue);
	Triangle wallLeft1(Vector(8, 8.1, -8.1), Vector(8, -4.1, -8.1), Vector(8, 8.1, 8.1), color_winered);
	Triangle wallLeft2(Vector(8, 8.1, 8.1), Vector(8, -4.1, -8.1), Vector(8, -4.1, 8.1), color_winered);
	Sphere sphere(Vector(-0, 0, 0), 3, color_perfectMirror);
	Sphere sphere2(Vector(-3, 2, -6.5), 1, color_green);
	Sphere sphere3(Vector(5, 1, 1), 2, color_winered2);
	Sphere sphere4(Vector(7, 1, -10), 2, color_winered);

	objects.push_back(&ground1);
	objects.push_back(&ground2);
	objects.push_back(&wallLeft1);
	objects.push_back(&wallLeft2);
	objects.push_back(&wallFront1);
	objects.push_back(&wallFront2);
	objects.push_back(&wallRight1);
	objects.push_back(&wallRight2);
	objects.push_back(&ceiling1);
	objects.push_back(&ceiling2);
	objects.push_back(&wallBack1);
	objects.push_back(&wallBack2);
	objects.push_back(&sphere);
	objects.push_back(&sphere2);
	objects.push_back(&sphere3);
	//objects.push_back(&sphere4);


	// images properties
	int imageWidth = 1024 * 6;
	int imageHeight = 1024 *6;
	int dpi = 72;
	int numOfPixels = imageHeight * imageWidth;

	// model light sources
	Light light(Vector(0, 7, 4), color_white);
	Camera camera = createCamera(0, 8, 15);
	vector<Source*> lights;
	lights.push_back(&light);
	t1 = clock();
	const int numThreads = 32;
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

