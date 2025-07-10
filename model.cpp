#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include "model.h"

Model::Model(const char *filename) : verts_(), faces_(), faces_texture_(), texture_verts_(), faces_normal_() {
    std::ifstream in;
    in.open (filename, std::ifstream::in);
    if (in.fail()) return;
    std::string line;
    while (!in.eof()) {
        std::getline(in, line);
        std::istringstream iss(line.c_str());
        char trash;
        if (!line.compare(0, 2, "v ")) {
            iss >> trash;
            Vec3f v;
            for (int i=0;i<3;i++) iss >> v[i];
            verts_.push_back(v);
        } else if (!line.compare(0, 2, "f ")) {
            std::vector<int> f;
            std::vector<int> ft;
            std::vector<int> fn;
            int nidx, idx, tidx;
            iss >> trash;
            while (iss >> idx >> trash >> tidx >> trash >> nidx) {
                idx--; // in wavefront obj all indices start at 1, not zero
                tidx--;
                nidx--;
                f.push_back(idx);
                ft.push_back(tidx);
                fn.push_back(nidx);
            }
            faces_.push_back(f);
            faces_texture_.push_back(ft);
            faces_normal_.push_back(fn);
        } else if (!line.compare(0, 3, "vt ")) {
            iss >> trash >> trash;
            Vec3f vt;
            for (int i=0;i<3;i++) iss >> vt[i];
            // 纹理图片坐标原点在左上角，需要转换为左下角
            vt[1] = 1 - vt[1];
            texture_verts_.push_back(vt);
        } else if (!line.compare(0, 3, "vn ")) {
            iss >> trash >> trash;
            Vec3f vn;
            for (int i=0;i<3;i++) iss >> vn[i];
            // vn.normalize();
            normal_vectors_.push_back(vn);
        }
        
    }
    std::cerr << "# v# " << verts_.size() << " f# "  << faces_.size() << " vt# "  << texture_verts_.size() << std::endl;
}

Model::~Model() {
}

int Model::nverts() {
    return (int)verts_.size();
}

int Model::nfaces() {
    return (int)faces_.size();
}

int Model::ntexture_verts() {
    return (int)texture_verts_.size();
}

int Model::nnormal_vectors()
{
    return (int)normal_vectors_.size();
}

std::vector<int> Model::face(int idx) {
    return faces_[idx];
}

std::vector<int> Model::face_texture(int idx)
{
    return faces_texture_[idx];
}

std::vector<int> Model::face_normal(int idx)
{
    return faces_normal_[idx];
}

Vec3f Model::vert(int i) {
    // printf("###获取顶点信息: %d\n",i);
    return verts_[i];
}

Vec3f Model::texture_vert(int i) {
    return texture_verts_[i];
}

Vec3f Model::normal_vector(int i)
{
    return normal_vectors_[i];
}
