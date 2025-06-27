#ifndef __MODEL_H__
#define __MODEL_H__

#include <vector>
#include "geometry.h"

class Model {
private:
	std::vector<Vec3f> verts_;
	std::vector<std::vector<int> > faces_;			// [][3]每个面三个顶点的索引
	std::vector<std::vector<int> > faces_texture_;	// [][3]每个面三个顶点的纹理索引
	std::vector<std::vector<int> > faces_normal_; 	// [][3]每个面三个顶点的法向量索引
	std::vector<Vec3f> texture_verts_;
	std::vector<Vec3f> normal_vectors_;
public:
	Model(const char *filename);
	~Model();
	int nverts();
	int nfaces();
	int ntexture_verts();
	int nnormal_vectors();
	Vec3f vert(int i);
	std::vector<int> face(int idx);
	std::vector<int> face_texture(int idx);
	std::vector<int> face_normal(int idx);
	Vec3f texture_vert(int i);
	Vec3f normal_vector(int i);
};

#endif //__MODEL_H__