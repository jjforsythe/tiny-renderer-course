#pragma once

#include <vector>
#include "geometry.h"
#include "tgaimage.h"

using namespace std;

class Model
{
private:
	vector<Vec3f> vertices;
	vector<Vec2f> vertices_tex;
	vector<vector<int>> faces;
	TGAImage diffusemap;

public:
	Model(const char *filename);
	virtual ~Model();
	int n_vertices();
	int n_faces();
	Vec3f vertex(int index);
	Vec2f vertex_tex(int index);
	vector<int> face(int index);
	void load_texture(std::string filename, const char *suffix, TGAImage &img);
	TGAColor diffuse(Vec2i uv);
};