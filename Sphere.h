#ifndef SPHERE_H
#define SPHERE_H

#include "Vector.h"
#include "Ray.h"
#include "Object.h"
#include "Colour.h"

class Sphere : public Object {

public:
	Vector center;
	double radius;
	Colour colour;
	
	Sphere(Vector c, double r, Colour col) : Object(col){
		center = c;
		radius = r;
		colour.red = col.red;
		colour.green = col.green;
		colour.blue = col.blue;
		colour.special = col.special;
	}

	Vector getNormalAt(Vector point) {
		// normal always points away from the center of a sphere
		Vector normal_Vect = point.vectAdd(center.negative()).normalize();
		return normal_Vect;
	}

	double intersect(Ray ray) {
		Vector ray_origin = ray.origin;
		double ray_origin_x = ray_origin.x;
		double ray_origin_y = ray_origin.y;
		double ray_origin_z = ray_origin.z;

		Vector ray_direction = ray.direction;
		double ray_direction_x = ray_direction.x;
		double ray_direction_y = ray_direction.y;
		double ray_direction_z = ray_direction.z;

		Vector sphere_center = this->center;
		double sphere_center_x = sphere_center.x;
		double sphere_center_y = sphere_center.y;
		double sphere_center_z = sphere_center.z;

		// quadaratic equation solution: http://www.siggraph.org/education/materials/HyperGraph/raytrace/rtinter1.htm

		double a = 1; // normalized
		double b = (2 * (ray_origin_x - sphere_center_x)*ray_direction_x) + (2 * (ray_origin_y - sphere_center_y)*ray_direction_y) + (2 * (ray_origin_z - sphere_center_z)*ray_direction_z);
		double c = pow(ray_origin_x - sphere_center_x, 2) + pow(ray_origin_y - sphere_center_y, 2) + pow(ray_origin_z - sphere_center_z, 2) - (radius*radius);

		double discriminant = b*b - 4 * c;		// solution of the quadratic sphere equation (root1/2)

		if (discriminant > 0) {
			// the ray intersects the sphere

			// the first root
			double root_1 = ((-1 * b - sqrt(discriminant)) / 2) - 0.000001;

			if (root_1 > 0) {
				// the first root is the smallest positive root
				return root_1;
			}
			else {
				// the second root is the smallest positive root
				double root_2 = ((sqrt(discriminant) - b) / 2) - 0.000001;
				return root_2;
			}
		}
		else {
			// the ray missed the sphere
			return -1;
		}
	}
};

#endif
