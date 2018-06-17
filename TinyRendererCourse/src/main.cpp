#include <iostream>
#include <algorithm>
#include "tgaimage.h"
#include "Model.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
const TGAColor green = TGAColor(0, 255, 0, 255);
const TGAColor blue = TGAColor(0, 0, 255, 255);

Model *model = NULL;

const int width = 3000;
const int height = 3000;

const Vec3f camera(0, 0, 3);
const Vec3f light_dir(0, 0, -1);

const bool RENDER_WIRE_MESH = true;

const char* filename = "output/render.tga";

// based on lessons here: https://github.com/ssloy/tinyrenderer

bool sortByAscendingY(const Vec2i &v0, const Vec2i &v1) {
	return v0.y < v1.y;
}

Vec3f cross(Vec3f x1, Vec3f x2) {
	return Vec3f((x1.y*x2.z - x1.z*x2.y), (x1.z*x2.x - x1.x*x2.z), (x1.x*x2.y - x1.y*x2.x));
}

Vec3f barycentric(Vec3f A, Vec3f B, Vec3f C, Vec3f P) {
	Vec3f s[2];
	for (int i = 2; i--; ) {
		s[i][0] = C[i] - A[i];
		s[i][1] = B[i] - A[i];
		s[i][2] = A[i] - P[i];
	}
	Vec3f u = cross(s[0], s[1]);
	if (std::abs(u[2])>1e-2) // dont forget that u[2] is integer. If it is zero then triangle ABC is degenerate
		// This output represents (1 - u - v, v, u)
		return Vec3f(1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);
	return Vec3f(-1, 1, 1); // in this case generate negative coordinates, it will be thrown away by the rasterizator
}

void line(Vec2i v0, Vec2i v1, TGAImage &image, TGAColor color) {

	bool steep = false;
	if (std::abs(v0.x - v1.x) < std::abs(v0.y - v1.y)) {
		std::swap(v0.x, v0.y);
		std::swap(v1.x, v1.y);
		steep = true;
	}
	if (v1.x < v0.x) { // make it left-to-right	
		std::swap(v0.x, v1.x);
		std::swap(v0.y, v1.y);
	}

	int dx = v1.x - v0.x;
	int dy = v1.y - v0.y;
	int derror2 = std::abs(dy) * 2;
	int error2 = 0;
	int y = v0.y;

	for (int x = v0.x; x <= v1.x; x++) {
		if (steep)
			image.set(y, x, color);
		else
			image.set(x, y, color);
		error2 += derror2;
		if (error2 > dx) {
			y += (v1.y > v0.y ? 1 : -1);
			error2 -= dx * 2;
		}
	}
}

void triangle(Vec3f *pts, Vec2f *uv, float *zbuffer, TGAImage &image, float intensity) {
	Vec2f bboxmin(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
	Vec2f bboxmax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
	Vec2f clamp(image.get_width() - 1, image.get_height() - 1);
	for (int i = 0; i<3; i++) {
		for (int j = 0; j<2; j++) {
			bboxmin[j] = std::max(0.f, std::min(bboxmin[j], pts[i][j]));
			bboxmax[j] = std::min(clamp[j], std::max(bboxmax[j], pts[i][j]));
		}
	}
	Vec3f P;
	for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++) {
		for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++) {
			Vec3f bc_screen = barycentric(pts[0], pts[1], pts[2], P);
			if (bc_screen.x<0 || bc_screen.y<0 || bc_screen.z<0) continue;
			P.z = 0;
			for (int i = 0; i<3; i++) P.z += pts[i][2] * bc_screen[i];
			if (zbuffer[int(P.x + P.y*width)]<P.z) {
				zbuffer[int(P.x + P.y*width)] = P.z;

				Vec2f uvt = Vec2f(uv[0].x + bc_screen.z * (uv[2]-uv[0]).x + bc_screen.y * (uv[1] - uv[0]).x,
					uv[0].y + bc_screen.z * (uv[2] - uv[0]).y + bc_screen.y * (uv[1] - uv[0]).y);

				TGAColor color = model->diffuse(Vec2i(uvt.x * 1024.f, uvt.y * 1024.f));
				color = TGAColor(color.r * intensity, color.g * intensity, color.b * intensity, color.a * intensity);
				image.set(P.x, P.y, color);
			}
		}
	}
}

Vec3f world2screen(Vec3f v) {
	return Vec3f(int((v.x + 1.) * (width / 2.)), int((v.y + 1.) * (height / 2.)), int((v.z + 1.) * (width / 2.)));
}

Vec3f projection(Vec3f x) {
	float c = camera.z;
	float divisor = 1. - (x.z / c);;

	return Vec3f(x.x / divisor, x.y / divisor, x.z / divisor);
}

int main(int argc, char** argv) {
	model = new Model("res/models/african_head.obj");

	TGAImage image(width, height, TGAImage::RGB);

	float *zbuffer = new float[width*height];
	for (int i = width * height; i--; zbuffer[i] = -std::numeric_limits<float>::max());

	for (int i = 0; i<model->n_faces(); i++) {
		std::vector<int> face = model->face(i);
		
		Vec3f v0 = projection(model->vertex(face[0]));
		Vec3f v1 = projection(model->vertex(face[1]));
		Vec3f v2 = projection(model->vertex(face[2]));

		Vec3f pts[3];
		Vec2f uv[3];
		for (int j = 0; j < 3; j++) {
			pts[j] = world2screen(projection(model->vertex(face[j])));
			uv[j] = model->vertex_tex(face[j + 3]);
		}

		Vec3f n = cross(pts[0] - pts[2], pts[0] - pts[1]);
		n.normalize();

		float intensity = n * light_dir;
		if (intensity > 0)
			triangle(pts, uv, zbuffer, image, intensity);

		if (RENDER_WIRE_MESH) {
			for (int j = 0; j < 3; j++) {
				line(Vec2i(pts[j].x, pts[j].y), Vec2i(pts[(j + 1) % 3].x, pts[(j + 1) % 3].y), image, green);
			}
		}
	}
	
	image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
	image.write_tga_file(filename);

	return 0;
}