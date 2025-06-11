#include "tgaimage.h"
#include "model.h"
#include "geometry.h"

#include <vector>
#include <cmath>

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0,   0,   255);

Model *model = NULL;
const int width  = 800;
const int height = 800;

void line(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor const &color) { 
    bool steep = false; 
    if (std::abs(x0-x1)<std::abs(y0-y1)) { 
        std::swap(x0, y0); 
        std::swap(x1, y1); 
        steep = true; 
    } 
    if (x0>x1) { 
        std::swap(x0, x1); 
        std::swap(y0, y1); 
    } 
    int dx = x1-x0; 
    int dy = y1-y0; 
    int derror2 = std::abs(dy)*2; 
    int error2 = 0; 
    int y = y0; 
    if(steep) {
        for(int x = x0; x<=x1; ++x) {
            image.set(y, x, color);
            error2 += derror2;
            if(error2 > dx) {
                y += (y1>y0? 1 : -1);
                error2 -= dx*2;
            }
        }
    } else {
        for(int x = x0; x<=x1; ++x) {
            image.set(x, y, color);
            error2 += derror2;
            if(error2 > dx) {
                y += (y1>y0? 1 : -1);
                error2 -= dx*2;
            }
        }
    }
} 

void triangle(Vec2i p1, Vec2i p2, Vec2i p3, TGAImage &image, TGAColor const &color) {
    int l = std::min(p1.x, std::min(p2.x, p3.x));
    int r = std::max(p1.x, std::max(p2.x, p3.x));
    int b = std::min(p1.y, std::min(p2.y, p3.y));
    int t = std::max(p1.y, std::max(p2.y, p3.y));

    Vec2i e1 = p2 - p1;
    Vec2i e2 = p3 - p2;
    Vec2i e3 = p1 - p3;

    Vec2i p(l,b);
    for(; p.x<=r; p.x++){
        p.y = b;
        for(; p.y<=t; p.y++){
            Vec2i ep1 = p - p1;
            Vec2i ep2 = p - p2;
            Vec2i ep3 = p - p3;
            int f1 = e1^ep1;
            int f2 = e2^ep2;
            int f3 = e3^ep3;
            if((f1>=0&&f2>=0&&f3>=0)||(f1<=0&&f2<=0&&f3<=0)){
                image.set(p.x, p.y, color);
            }
        }
    }
}

int main(int argc, char** argv) {
	if (2==argc) {
        model = new Model(argv[1]);
    } else {
        model = new Model("obj/african_head.obj");
    }

    TGAImage image(width, height, TGAImage::RGB);
    // for (int i=0; i<model->nfaces(); i++) {
    //     std::vector<int> face = model->face(i);
    //     for (int j=0; j<3; j++) {
    //         Vec3f v0 = model->vert(face[j]);
    //         Vec3f v1 = model->vert(face[(j+1)%3]);
    //         int x0 = (v0.x+1.)*width/2.;
    //         int y0 = (v0.y+1.)*height/2.;
    //         int x1 = (v1.x+1.)*width/2.;
    //         int y1 = (v1.y+1.)*height/2.;
    //         line(x0, y0, x1, y1, image, white);
    //     }
    // }

    triangle(Vec2i(100,100),Vec2i(200,400),Vec2i(400,100), image, white);

    image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
    image.write_tga_file("output.tga");
    delete model;
	std::cout<<"图片成功生成！"<<std::endl;
	return 0;
}

