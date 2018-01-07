#pragma once
#include "Object.h"
#include "Vector.h"
class Triangle :
	public Object
{
public:
	Triangle();
	~Triangle();

	Vector vertex0;
	Vector vertex1;
	Vector vertex2;
	Colour colour;
	Vector normal;

	Triangle(Vector v0, Vector v1, Vector v2, Colour col) : Object(col) {
		vertex0 = v0;
		vertex1 = v1;
		vertex2 = v2;
		colour = col;
	}


	double intersect(Ray ray,
		Vector& outIntersectionPoint)
	{
		const double EPSILON = 0.0000001;
		Vector vertex0 = this->vertex0;
		Vector vertex1 = this->vertex1;
		Vector vertex2 = this->vertex2;
		Vector edge1, edge2, h, s, q;
		double a, f, u, v;
		edge1 = vertex1.vectSub(vertex0);
		edge2 = vertex2.vectSub(vertex0);
		h = ray.direction.crossProduct(edge2);
		a = edge1.dotProduct(h);
		if (a > -EPSILON && a < EPSILON)
			return -1;
		f = 1 / a;
		s = ray.origin.vectSub(vertex0);
		u = f * (s.dotProduct(h));
		if (u < 0.0 || u > 1.0)
			return -1;
		q = s.crossProduct(edge1);
		v = f * ray.direction.dotProduct(q);
		if (v < 0.0 || u + v > 1.0)
			return -1;
		// At this stage we can compute t to find out where the intersection point is on the line.
		double t = f * edge2.dotProduct(q);
		if (t > EPSILON) // ray intersection
		{
			outIntersectionPoint = ray.origin.vectAdd(ray.direction.vectMult(t));
			return 1;
		}
		else // This means that there is a line intersection but not a ray intersection.
			return -1;
	}

	void setNormal(Vector normal)
	{
		this->normal = normal;
	}

	Vector getNormalAt(Vector intersection_position, Vector intersection_to_light_direction) {
		Vector e0 = vertex1.vectSub(vertex0);
		Vector e1 = vertex2.vectSub(vertex0);
		Vector normal = e0.crossProduct(e1).normalize(); // this is the triangle's normal
		if (normal.dotProduct(intersection_to_light_direction) <= 0)
			return normal.negative();
		return normal;
	}

	double getDistance(Vector to) {
		return to.vectSub(vertex0).magnitude();
	}
};

