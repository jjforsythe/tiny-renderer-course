#include <iostream>
#include <fstream>
#include <sstream>
#include "model.h"

using namespace std;

Model::Model(const char *filename)
{
	ifstream in;
	in.open(filename);
	if (in.fail()) {
		cout << "File " + string(filename) + " not found" << endl;
		system("PAUSE");
		return;
	}

	load_texture(filename, "_diffuse.tga", diffusemap);

	string line;

	while (!in.eof()) {
		getline(in, line);
		istringstream iss(line.c_str());
		char trash;

		if (!line.compare(0, 2, "v ")) {
			iss >> trash;
			Vec3f v;
			for (int i = 0; i < 3; i++) iss >> v[i];
			vertices.push_back(v);
		}
		else if (!line.compare(0, 3, "vt ")) {
			iss >> trash;
			iss >> trash;
			Vec2f vt;
			for (int i = 0; i < 2; i++) iss >> vt[i];
			vertices_tex.push_back(vt);
		}
		else if (!line.compare(0, 2, "f ")) {
			// f for normal vertices, t for texture vertices
			vector<int> f;
			vector<int> t;
			int itrash, idx, idx_t;
			iss >> trash;
			while (iss >> idx >> trash >> idx_t >> trash >> itrash) {
				idx--;
				f.push_back(idx);
				idx_t--;
				t.push_back(idx_t);
			}
			for (int i = 0; i < t.size(); i++)
				f.push_back(t[i]);
			faces.push_back(f);
		}
	}
	cout << "# vertices: " << n_vertices() << endl;
	cout << "# faces: " << n_faces() << endl;
}

Model::~Model()
{
}

int Model::n_vertices() {
	return (int)vertices.size();
}

int Model::n_faces() {
	return (int)faces.size();
}

Vec3f Model::vertex(int index) {
	return vertices[index];
}

Vec2f Model::vertex_tex(int index) {
	return vertices_tex[index];
}

vector<int> Model::face(int index) {
	return faces[index];
}

void Model::load_texture(string filename, const char *suffix, TGAImage &img) {
	string texfile(filename);
	size_t dot = texfile.find_last_of(".");
	if (dot != string::npos) {
		texfile = texfile.substr(0, dot) + string(suffix);
		cerr << "texture file " << texfile << " loading " << (img.read_tga_file(texfile.c_str()) ? "" : "failed\n");
		img.flip_vertically();
	}
}

TGAColor Model::diffuse(Vec2i uv) {
	return diffusemap.get(uv.x, uv.y);
}
