#include "BMP\Bitmap.h"
#include "BMP\BMPHandler.h"
#include "Camera.h"

#include <fstream>
#include <cmath>
#include <iostream>

#include "Vector.h"

using namespace std;

class Ray {
public:
	Vector origin;
	Vector direction;
	Ray(Vector orig, Vector dir) {
		origin = orig;
		direction = dir.normalize();
	}
};

class Sphere {
public:
	Vector center;
	double radius;
	Sphere(Vector c, double r) {
		center = c;
		radius = r;
	}

	bool findIntersection(Ray ray) {
		Vector ray_origin = ray.origin;
		double ray_origin_x = ray_origin.x;
		double ray_origin_y = ray_origin.y;
		double ray_origin_z = ray_origin.z;

		Vector ray_direction = ray.origin;
		double ray_direction_x = ray_direction.x;
		double ray_direction_y = ray_direction.y;
		double ray_direction_z = ray_direction.z;

		Vector sphere_center = center;
		double sphere_center_x = sphere_center.x;
		double sphere_center_y = sphere_center.y;
		double sphere_center_z = sphere_center.z;

		double a = 1; // normalized
		double b = (2 * (ray_origin_x - sphere_center_x)*ray_direction_x) + (2 * (ray_origin_y - sphere_center_y)*ray_direction_y) + (2 * (ray_origin_z - sphere_center_z)*ray_direction_z);
		double c = pow(ray_origin_x - sphere_center_x, 2) + pow(ray_origin_y - sphere_center_y, 2) + pow(ray_origin_z - sphere_center_z, 2) - (radius*radius);

		double discriminant = b*b - 4 * c;

		if (discriminant > 0) {
			/// the ray intersects the sphere

			// the first root
			double root_1 = ((-1 * b - sqrt(discriminant)) / 2) - 0.000001;

			if (root_1 > 0) {
				// the first root is the smallest positive root
				return true;
			}
			else {
				// the second root is the smallest positive root
				double root_2 = ((sqrt(discriminant) - b) / 2) - 0.000001;
				return false;
			}
		}
		else {
			// the ray missed the sphere
			return false;
		}
	}



	bool intersect(Ray ray, Vector& S) {
		Vector m = center;
		Vector o = ray.origin;
		Vector om = o - m;
		double l = abs(om * ray.direction);
		double h2squared = pow(om.abs(), 2) - l*l;
		double hsquared = radius*radius - h2squared;

		if (h2squared <= radius*radius) {
			if (pow(om.abs(), 2) >= radius*radius) //not inside
			{
				//if (l < 0)
					//return false;
				S = o + ray.direction * (l - sqrt(hsquared));
				//cout << S.toString() << endl;
			}
			else {
				S = o + ray.direction * (l + sqrt(hsquared));
				//cout << S.toString() << endl;
			}
			return true;
		}
		return false;
	}
};

int main(int argc, char** argv)
{
	int imageHeight = 500;
	int imageWidth = 500;
	Vector eye(250, 250, 1);
	Vector light(3, 3, -4);
	Vector S;


	Vector O(0, 0, 0);
	Vector X(1, 0, 0);
	Vector Y(0, 1, 0);
	Vector Z(0, 0, 1);

	Vector new_sphere_location(1.75, -0.25, 0);
	Sphere sphere(new_sphere_location, 1);

	Vector campos(3, 1.5, -4);

	Vector look_at(0, 0, 0);
	Vector diff_btw(campos.x - look_at.x, campos.y - look_at.y, campos.z - look_at.z);

	Vector camdir = diff_btw.negative().normalize();
	Vector camright = Y.crossProduct(camdir).normalize();
	Vector camdown = camright.crossProduct(camdir);
	Camera scene_cam(campos, camdir, camright, camdown);





	double xamnt, yamnt;

	std::ofstream out("out.ppm");
	out << "P3\n" << imageWidth << ' ' << imageHeight << ' ' << "255\n";
	for (int height = 0; height < imageHeight; height++)
	{
		for (int width = 0; width < imageWidth; width++)
		{
			xamnt = (height + 0.5) / imageWidth;
			yamnt = ((imageHeight - width) + 0.5) / imageHeight;

			Vector cam_ray_origin = scene_cam.getCameraPosition();
			Vector cam_ray_direction = camdir+((camright*(xamnt - 0.5))+((camdown*(yamnt - 0.5)))).normalize();

			Ray ray(cam_ray_origin, cam_ray_direction);

			//Ray ray(campos, Vector((double)height, (double)width, 1) - campos);
			//Ray ray(Vector(width, height, 0), Vector(0, 0, 1));

			if (sphere.findIntersection(ray)) {
				//Ray shadow(S, light - S);
				//Vector S2;
				out << (int)255 << ' '
					<< (int)255 << ' '
					<< (int)255 << '\n';
				/*if (sphere.intersect(shadow, S2) && S2 != S) {
					cout << "Schattenfühler trifft erneut auf Objekt" << endl;
					out << (int)0 << ' '
						<< (int)0 << ' '
						<< (int)0 << '\n';
				}
				else {
					out << (int)255 << ' '
						<< (int)255 << ' '
						<< (int)255 << '\n';
				}*/
			}
			else {
				out << (int)0 << ' '
					<< (int)0 << ' '
					<< (int)0 << '\n';
			}
		}
	}
	return 0;
}
