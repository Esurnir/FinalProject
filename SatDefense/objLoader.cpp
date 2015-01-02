//inspired by http://www.opengl-tutorial.org/beginners-tutorials/tutorial-7-model-loading/

#include "objLoader.h"
#include <unordered_map>
#include <sstream>
#include <stdio.h>

MeshVBO meshLoad(const char* filename) {
	MeshVBO r;
	std::vector < glm::vec3 > tempVec, tempNormal;
	std::vector < glm::vec2 > tempTexCoord;
	std::unordered_map<std::string, unsigned int> elementMap;
	FILE* obj = fopen(filename, "r");
	if (obj == NULL){
		fprintf(stderr, "ERROR: Could not open file %s\n", filename);
		exit(1);
	}
	while (1){

		char lineHeader[128];
		// read the first word of the line
		int res = fscanf(obj, "%s", lineHeader);
		if (res == EOF)
			break; // EOF = End Of File. Quit the loop.

		// else : parse lineHeader
		if (strcmp(lineHeader, "v") == 0){
			glm::vec3 vertex;
			fscanf(obj, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
			tempVec.push_back(vertex);
		}
		else if (strcmp(lineHeader, "vn") == 0) {
			glm::vec3 normal;
			fscanf(obj, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
			tempNormal.push_back(normal);
		}
		else if (strcmp(lineHeader, "vt") == 0) {
			glm::vec2 tex;
			fscanf(obj, "%f %f\n", &tex.x, &tex.y);
			tex.y = 1 - tex.y; //DDS are flipped upside down, faster to flip the coordinate here than in the shader.
			tempTexCoord.push_back(tex);
		}

		else if (strcmp(lineHeader, "f") == 0) { // OK I'll cheat here, I know theorically I could have a malformed obj file, but in a blender export, faces indices are last, so I'll jump a step and process data right there.
			unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
			int matches = fscanf(obj, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
			if (matches != 9){
				fprintf(stderr, "File can't be read by our simple parser : ( Try exporting with other options\n");
				fclose(obj);
				exit(EXIT_FAILURE);
			}
			for (int i = 0; i < 3; ++i) {
				Vertex a;
				a.position = tempVec[vertexIndex[i] - 1];
				a.normal = tempNormal[normalIndex[i] - 1];
				a.texCoord = tempTexCoord[uvIndex[i] - 1];
				std::ostringstream  stringStream;
				stringStream << a.position.x << a.position.y << a.position.z << a.normal.x << a.normal.y << a.normal.z << a.texCoord.x << a.texCoord.y;
				std::string values = stringStream.str();
				std::unordered_map<std::string, unsigned int>::const_iterator it = elementMap.find(values);
				if (it == elementMap.end()) { //if that exact combination of position/normal/texCoord doesn't exist, create a new Vertex and insert its index in the map
					r.vertexBuffer.push_back(a);
					r.indexBuffer.push_back(r.vertexBuffer.size() - 1);
					elementMap[values] = r.vertexBuffer.size() - 1;
				}
				else { //else just insert the index to the object
					r.indexBuffer.push_back(it->second);
				}
			}
		}
	}
	fclose(obj);
	return r;
}

Mesh objLoad(const char* filename) {
	Mesh r;
	std::vector < Vec3 > tempVec,tempNormal;
	std::vector < Vec2 > tempTexCoord;
	FILE* obj = fopen(filename, "r");
	if (obj == NULL){
		fprintf(stderr, "ERROR: Could not open file %s\n", filename);
		exit(1);
	}
	int rsize = 0;
	while (1){

		char lineHeader[128];
		// read the first word of the line
		int res = fscanf(obj, "%s", lineHeader);
		if (res == EOF)
			break; // EOF = End Of File. Quit the loop.

		// else : parse lineHeader
		if (strcmp(lineHeader, "v") == 0){
			Vec3 vertex;
			fscanf(obj, "%f %f %f\n", &vertex.Position[0], &vertex.Position[1], &vertex.Position[2]);
			tempVec.push_back(vertex);
		}
		else if (strcmp(lineHeader, "vn") == 0) {
			Vec3 normal;
			fscanf(obj, "%f %f %f\n", &normal.Position[0], &normal.Position[1], &normal.Position[2]);
			tempNormal.push_back(normal);
		}
		else if (strcmp(lineHeader, "vt") == 0) {
			Vec2 tex;
			fscanf(obj, "%f %f\n",&tex.Position[0],&tex.Position[1]);
			tempTexCoord.push_back(tex);
		}
		
		else if (strcmp(lineHeader, "f") == 0) { // OK I'll cheat here, I know theorically I could have a malformed obj file, but in a blender export, faces indices are last, so I'll jump a step and process data right there.
			unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
			int matches = fscanf(obj, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
			if (matches != 9){
				fprintf(stderr, "File can't be read by our simple parser : ( Try exporting with other options\n");
				fclose(obj);
				exit(1);
			}
			for (int i = 0; i < 3; ++i) {
				Vec3 vec, norm;
				Vec2 tex;
				vec = tempVec[vertexIndex[i] - 1]; //I know, all that waste ! no Index ! but the problem is that I have a different ammount of normal, vertex and texcoord, and vbo need them to be packed, since I can't think of a way of checking if -that specific version- of a vertex already exist... this will do :-/
				norm = tempNormal[normalIndex[i] - 1];
				tex = tempTexCoord[uvIndex[i] - 1];
				r.Vertices.push_back(vec);
				r.Normals.push_back(norm);
				r.texCoord.push_back(tex);
			}
			rsize++;
		}
	}
	r.size = rsize;
	fclose(obj);
	return r;
}