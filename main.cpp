#include "tgaimage.h"
#include "model.h"
#include "geometry.h"

#include <vector>
#include <cmath>
#include <random>


Model *model = NULL;
const int width  = 800;
const int height = 800;
const int depth  = 255;

TGAColor RandomColor() {
    srand(int(time(0)));
    return TGAColor(rand()%256, rand()%256, rand()%256, rand()%256);
}

// 获取点P对于三角形ABC的中心坐标(1-u-v,u,v)，其中P=(1-u-v)A+uB+vC
Vec3f barycentric(Vec3f A, Vec3f B, Vec3f C, Vec3f P) {
    Vec3f s[2];
    for (int i=2; i--; ) {
        s[i][0] = C[i]-A[i];
        s[i][1] = B[i]-A[i];
        s[i][2] = A[i]-P[i];
    }
    Vec3f u = cross(s[0], s[1]);
    if (std::abs(u[2])>1e-2) // dont forget that u[2] is integer. If it is zero then triangle ABC is degenerate
        return Vec3f(1.f-(u.x+u.y)/u.z, u.y/u.z, u.x/u.z);
    return Vec3f(-1,1,1); // in this case generate negative coordinates, it will be thrown away by the rasterizator
}

// 在屏幕坐标(x0,y0)到(x1,y1)之间画线
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

TGAColor getColor(TGAImage &texture_image, Vec2f *texture_uv, Vec3f const &bc){
    int tex_x = texture_uv[0].x * bc.x + texture_uv[1].x * bc.y + texture_uv[2].x * bc.z;
    int tex_y = texture_uv[0].y * bc.x + texture_uv[1].y * bc.y + texture_uv[2].y * bc.z;
    TGAColor color = texture_image.get(tex_x, tex_y);
    return color;
}

// pts:三角形的三个顶点的屏幕坐标
// zbuffer:深度缓存
// texture_uv[3]:三角形的三个顶点的纹理坐标
// intensities[3]:三角形的三个顶点的光照强度
void triangle(Vec3f *pts, float *zbuffer, TGAImage &image, TGAImage &texture_image, Vec2f *texture_uv, float *intensities) {
    Vec2f bboxmin( std::numeric_limits<float>::max(),  std::numeric_limits<float>::max());
    Vec2f bboxmax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
    Vec2f clamp(image.get_width()-1, image.get_height()-1);
    for (int i=0; i<3; i++) {
        for (int j=0; j<2; j++) {
            bboxmin[j] = std::max(0.f,      std::min(bboxmin[j], pts[i][j]));
            bboxmax[j] = std::min(clamp[j], std::max(bboxmax[j], pts[i][j]));
        }
    }
    Vec3f P;
    for (P.x=bboxmin.x; P.x<=bboxmax.x; P.x++) {
        for (P.y=bboxmin.y; P.y<=bboxmax.y; P.y++) {
            Vec3f bc_screen  = barycentric(pts[0], pts[1], pts[2], P);
            if (bc_screen.x<0 || bc_screen.y<0 || bc_screen.z<0) continue;
            P.z = 0;
            for (int i=0; i<3; i++) P.z += pts[i][2]*bc_screen[i];
            // 获取颜色
            TGAColor color = getColor(texture_image,texture_uv,bc_screen);
            // printf("intensity: %f\n",intensity);
            float intensity = intensities[0] * bc_screen.x + intensities[1] * bc_screen.y + intensities[2] * bc_screen.z;
            color = color * intensity;
            // TGAColor color = colors[2];
            if (zbuffer[int(P.x+P.y*width)]<P.z) {
                zbuffer[int(P.x+P.y*width)] = P.z;
                image.set(P.x, P.y, color);
            }
        }
    }
}

// 世界坐标转换为屏幕坐标
Vec3f world2screen(Vec3f v) {
    return Vec3f(int((v.x+1.)*width/2.+.5), int((v.y+1.)*height/2.+.5), v.z);
}

TGAColor avg_color(TGAColor *colors) {
    int bgra[4]={0,0,0,0};
    // j: index of bgra
    for(int j=0;j<4;j++){
        // i: i th of 3 colors
        for(int i=0;i<3;i++) {
            bgra[j] += colors[i][j];
        }
        bgra[j]/=3;
    }
    return TGAColor(bgra[2],bgra[1],bgra[0],bgra[3]);
}

int main(int argc, char** argv) {
	if (2==argc) {
        model = new Model(argv[1]);
    } else {
        model = new Model("obj/african_head.obj");
    }
    TGAImage texture_image;
    texture_image.read_tga_file("obj/african_head_diffuse.tga");

    // for(int i=0;i<model->ntexture_verts();i++){
    //     printf("#####%f,%f,%f\n",model->texture_vert(i).x,model->texture_vert(i).y,model->texture_vert(i).z);
    // }

    TGAImage image(width, height, TGAImage::RGB);

    // 三角面渲染
    // 定义方向光光源
    Vec3f light_dir(0,-1,-1); 
    light_dir.normalize();

    float *zbuffer = new float[width*height]; 
    for (int i=width*height; i--; zbuffer[i] = -std::numeric_limits<float>::max());

    for (int i=0; i<model->nfaces(); i++) { // 每个循环处理一个三角面
        std::vector<int> face = model->face(i);             // 三个顶点的索引
        std::vector<int> texture = model->face_texture(i);  // 三个顶点的纹理索引
        std::vector<int> normal = model->face_normal(i);    // 三个顶点的法向量索引

        Vec3f pts[3];           // 三个顶点的屏幕坐标
        Vec3f world_coords[3];  // 三个顶点的世界坐标
        Vec2f texture_uv[3];    // 三个顶点的纹理坐标
        for (int j=0; j<3; j++){
            pts[j] = world2screen(model->vert(face[j]));
            world_coords[j]  = model->vert(face[j]);
            int tidx = texture[j];
            Vec3f texture_point = model->texture_vert(tidx);
            texture_uv[j].x = std::min(float(texture_image.get_width()-1.0), (texture_point.x * texture_image.get_width()));
            texture_uv[j].y = std::min(float(texture_image.get_height()-1.0), (texture_point.y * texture_image.get_height()));
        }
        // 计算三个顶点的光照强度
        float intensities[3];
        for(int j=0;j<3;j++){
            Vec3f n = model->normal_vector(normal[j]);
            n.normalize();
            intensities[j] = -(n*light_dir); // 法向量与光照强度的点积取相反数为光照强度
            if(intensities[j] < 0)intensities[j] = 0;
        }
        triangle(pts, zbuffer, image, texture_image, texture_uv, intensities);
    }

    // 图片输出
    image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
    image.write_tga_file("output.tga");
    delete model;
	std::cout<<"图片成功生成！"<<std::endl;
	return 0;
}

