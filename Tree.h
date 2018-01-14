#pragma once
#include  <vector>
#include "Vector.h"
#include "Box.h"

using namespace std;


class Tree {
public:
	vector <Object*> triangles;
	vector <Tree* > children;
	Box* myBox;
	Tree(int depth, int maxdepth, Vector min, Vector max) {
		//this->depth = depth;
		this->myBox = new Box(min, max);
		if (depth < maxdepth) {
			depth++;
			for (int x = 0; x < 2; x++) {
				for (int y = 0; y < 2; y++) {
					for (int z = 0; z < 2; z++) {
						float diffX = abs(min.x - max.x);
						float diffY = abs(min.y - max.y);
						float diffZ = abs(min.z - max.z);
						Vector maxtemp = Vector(max.x - 0.5 * diffX, max.y - 0.5 * diffY, max.z - 0.5 * diffZ);
						Vector minNew = Vector(min.x + (0.5)*float(x)*diffX, min.y + (0.5)*float(y)*diffY, min.z + (0.5)*float(z)*diffZ);
						Vector maxNew = Vector(maxtemp.x + (0.5)*float(x)*diffX, maxtemp.y + (0.5)*float(y)*diffY, maxtemp.z + (0.5)*float(z)*diffZ);
						children.push_back(new Tree(depth, maxdepth, minNew.vectMult(1), maxNew.vectMult(1)));
					}
				}
			}
		}
	}

	void assignTriangles(vector <Object*> triangles)
	{
		for (Object * tr : triangles)
		{
			float distance = INFINITY;
			Tree* closest = nullptr;
			for (Tree * T : children)
			{
				for (Tree* T2 : T->children) {
					for (Tree* T3 : T2->children) {
						for (Tree* T4 : T3->children) {
							if (tr->getDistance(T4->myBox->center) < distance)
							{
								distance = tr->getDistance(T4->myBox->center);
								closest = T4;
							}
						}
					}
				}
			}
			closest->triangles.push_back(tr);
		}
	}

	vector<Object*> findrelevantObjects(Ray ray, vector<Object*> objectBackup)
	{
		vector <Object*> thisItObjects;

		if (this->myBox->intersect(ray))
			for (Tree* T2 : this->children) {
				if (T2->myBox->intersect(ray))
					for (Tree* T3 : T2->children) {
						if (T3->myBox->intersect(ray))
							for (Tree* T4 : T3->children) {
								if (T4->myBox->intersect(ray))
									for (Tree* T5 : T4->children) {
										if (T5->myBox->intersect(ray)) {
											thisItObjects.insert(std::end(thisItObjects), std::begin(T5->triangles), std::end(T5->triangles));
										}
									}
							}
					}
			}
		thisItObjects.insert(std::end(thisItObjects), std::begin(objectBackup), std::end(objectBackup));
		return thisItObjects;
	}
};



