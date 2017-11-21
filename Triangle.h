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

	Triangle(Vector v0, Vector v1, Vector v2, Colour col) : Object(col) {
		vertex0 = v0;
		vertex1 = v1;
		vertex2 = v2;
		colour = col;
	}


	double intersect(Ray ray,
		Vector& outIntersectionPoint)
	{
		const float EPSILON = 0.0000001;
		Vector vertex0 = this->vertex0;
		Vector vertex1 = this->vertex1;
		Vector vertex2 = this->vertex2;
		Vector edge1, edge2, h, s, q;
		float a, f, u, v;
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
		float t = f * edge2.dotProduct(q);
		if (t > EPSILON) // ray intersection
		{
			outIntersectionPoint = ray.origin.vectAdd(ray.direction.vectMult(t));
			return 1;
		}
		else // This means that there is a line intersection but not a ray intersection.
			return -1;
	}


	Vector getNormalAt(Vector intersection_position, Vector intersection_to_light_direction) {
		Vector normal = vertex1.crossProduct(vertex2).normalize(); // this is the triangle's normal
		if (normal.dotProduct(intersection_to_light_direction) < 0)
			return normal.negative();
		return normal;
	}

	static bool RayIntersectsTriangle(Vector rayOrigin,
		Vector rayVector,
		Triangle* inTriangle,
		Vector& outIntersectionPoint)
	{
		const float EPSILON = 0.0000001;
		Vector vertex0 = inTriangle->vertex0;
		Vector vertex1 = inTriangle->vertex1;
		Vector vertex2 = inTriangle->vertex2;
		Vector edge1, edge2, h, s, q;
		float a, f, u, v;
		edge1 = vertex1.vectSub(vertex0);
		edge2 = vertex2.vectSub(vertex0);
		h = rayVector.crossProduct(edge2);
		a = edge1.dotProduct(h);
		if (a > -EPSILON && a < EPSILON)
			return false;
		f = 1 / a;
		s = rayOrigin.vectSub(vertex0);
		u = f * (s.dotProduct(h));
		if (u < 0.0 || u > 1.0)
			return false;
		q = s.crossProduct(edge1);
		v = f * rayVector.dotProduct(q);
		if (v < 0.0 || u + v > 1.0)
			return false;
		// At this stage we can compute t to find out where the intersection point is on the line.
		float t = f * edge2.dotProduct(q);
		if (t > EPSILON) // ray intersection
		{
			outIntersectionPoint = rayOrigin.vectAdd(rayVector.vectMult(t));
			return true;
		}
		else // This means that there is a line intersection but not a ray intersection.
			return false;
	}
};

