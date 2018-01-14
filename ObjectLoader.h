#ifndef ObjLoader_H
#define ObjLoader_H

#include <vector>
#include "Vector.h"
#include "Vector2.h"

class ObjectLoader {

public:

	bool readFile(const char * path, std::vector <Vector> & out_vertices, std::vector <Vector2> & out_uvs, std::vector <Vector> & out_normals) {
		std::vector< unsigned int > vertexIndices, uvIndices, normalIndices;
		std::vector< Vector > temp_vertices;
		std::vector< Vector2 > temp_uvs;
		std::vector< Vector > temp_normals;

		FILE * file = fopen(path, "r");
		if (file == NULL) {
			printf("Impossible to open the file !\n");
			return false;
		}

		while (1) {

			char lineHeader[128];
			// read the first word of the line
			int res = fscanf(file, "%s", lineHeader);
			if (res == EOF)
				break; // EOF = End Of File. Quit the loop.

			// else : parse lineHeader
			if (strcmp(lineHeader, "v") == 0) {
				float x,y,z = 0;
				fscanf(file, "%f %f %f\n", &x, &y, &z);
				Vector vertex(x-2, y+1, z);
				//Vector vertex(x, y, z);
				//vertex = vertex.vectMult(2);
				temp_vertices.push_back(vertex);
			}
			else if (strcmp(lineHeader, "vt") == 0) {
				Vector2 uv;
				fscanf(file, "%f %f\n", &uv.x, &uv.y);
				temp_uvs.push_back(uv);
			}
			else if (strcmp(lineHeader, "vn") == 0) {
				float x, y, z = 0;
				fscanf(file, "%f %f %f\n", &x, &y, &z);
				Vector normal(x, y, z);
				temp_normals.push_back(normal);
			}
			else if (strcmp(lineHeader, "f") == 0) {
				std::string vertex1, vertex2, vertex3;
				unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
				int matches = fscanf(file, "%d//%d %d//%d %d//%d\n", &vertexIndex[0], &normalIndex[0], &vertexIndex[1], &normalIndex[1], &vertexIndex[2], &normalIndex[2]);
				if (matches != 6) {
					printf("File can't be read by our simple parser : ( Try exporting with other options\n");
					return false;
				}
				vertexIndices.push_back(vertexIndex[0]);
				vertexIndices.push_back(vertexIndex[1]);
				vertexIndices.push_back(vertexIndex[2]);
				//uvIndices.push_back(uvIndex[0]);
				//uvIndices.push_back(uvIndex[1]);
				//uvIndices.push_back(uvIndex[2]);
				normalIndices.push_back(normalIndex[0]);
				normalIndices.push_back(normalIndex[1]);
				normalIndices.push_back(normalIndex[2]);

				
			}
		}
		// For each vertex of each triangle
		for (unsigned int i = 0; i < vertexIndices.size(); i++) {
			unsigned int vertexIndex = vertexIndices[i];
			Vector vertex = temp_vertices[vertexIndex - 1];
			out_vertices.push_back(vertex);
		}

		for (unsigned int i = 0; i < normalIndices.size(); i++) {
			unsigned int normalIndex = normalIndices[i];
			Vector normal = temp_normals[normalIndex - 1];
			out_normals.push_back(normal);
		}

	}

};

#endif ObjLoader_H