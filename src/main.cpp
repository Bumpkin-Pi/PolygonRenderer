#include <iostream>
#include <cstdio>
#include <SDL2/SDL.h>
#include <cmath>
#include <vector>
#include <Eigen/Dense>

#include "species.h"

using Eigen::MatrixXf;


const int screen_width = 1000;
const int screen_height = 1000;
SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;

bool running = true;
bool paused = false;



struct Camera{
    float x;
    float y;
    float z;
    float roll; // Roll
    float pitch; // pitch
    float yaw; // Yaw
    float FovX;
    float FovY;
};
Camera camera1;
species::Polygon polygon1;
species::Polygon polygon2;

void dothething(){
    Eigen::AngleAxisd rollAngle(camera1.roll, Eigen::Vector3d::UnitZ());
    Eigen::AngleAxisd yawAngle(camera1.yaw, Eigen::Vector3d::UnitY());
    Eigen::AngleAxisd pitchAngle(camera1.pitch, Eigen::Vector3d::UnitX());

    Eigen::Quaternion<double> q = rollAngle * yawAngle * pitchAngle;

    Eigen::Matrix3d rotationMatrix = q.matrix();
    std::cout << rotationMatrix << std::endl;
}


namespace matrixShite{
    float left = -3;
    float right = 3;
    float bottom = -3;
    float top = 3;
    float near = 5;
    float far = 6;

    MatrixXf getMorth(){
        MatrixXf Morth(4,4); Morth.setZero();

        Morth(0, 0) = 2/(right-left);
        Morth(0, 3) = -(right + left) / (right - left);
        Morth(1, 1) = 2/(top - bottom);
        Morth(1, 3) = -(top + bottom) / (top - bottom);
        Morth(2, 2) = -2 / (far - near);
        Morth(2, 3) = -(far + near) / (far - near);
        Morth(3, 3) = 1;

        return Morth;
    }

    MatrixXf getMp2o(){
        MatrixXf Mp2o(4, 4); Mp2o.setZero();
        Mp2o(0, 0) = near;
        Mp2o(1, 1) = near;
        Mp2o(2, 2) = far + near;
        Mp2o(2, 3) = far * near;
        Mp2o(3, 2) = -1;
        return Mp2o;
    }

    MatrixXf getMvp(){
        MatrixXf Mvp(3, 4); Mvp.setZero();
        Mvp(0, 0) = screen_width / 2;
        Mvp(0, 3) = (screen_width - 1) / 2;
        Mvp(1, 1) = screen_height / 2;
        Mvp(1, 3) = (screen_height - 1) / 2;
        Mvp(2, 2) = 0.5;
        Mvp(2, 3) = 0.5;
        return Mvp;
    }

    MatrixXf getMper(){
        MatrixXf Mper(4, 4); Mper.setZero();
        Mper = getMorth() * getMp2o();

//        Mper(0, 0) = (2 * near) / (right - left);
//        Mper(0, 2) = (right + left) / (right - left);
//        Mper(1, 1) = (2 * near) / (top - bottom);
//        Mper(1, 2) = (top + bottom) / (top - bottom);
//        Mper(2, 2) = -(far + near) / (far - near);
//        Mper(2, 3) = -(2 * far * near) / (far - near);
//        Mper(3, 2) = -(2 * far * near) / (far - near);

        return Mper;
    }

    MatrixXf getMcam(){ // I do not trust this function in the slightest
        MatrixXf cam(4, 4); cam.setZero();
        MatrixXf rotation(4, 4); rotation.setZero();
        MatrixXf translation(4, 4); translation.setZero();

        // Translation matrix
        translation(0, 3) = -camera1.x;
        translation(1, 3) = -camera1.y;
        translation(2, 3) = -camera1.z;
        translation(0, 0) = 1;
        translation(1, 1) = 1;
        translation(2, 2) = 1;
        translation(3, 3) = 1;

        // Rotation matrix
        // Translation matrix
        translation(0, 3) = -camera1.x;
        translation(1, 3) = -camera1.y;
        translation(2, 3) = -camera1.z;
        translation(0, 0) = 1;
        translation(1, 1) = 1;
        translation(2, 2) = 1;
        translation(3, 3) = 1;

        // Rotation matrix

        Eigen::AngleAxisd rollAngle(camera1.roll, Eigen::Vector3d::UnitZ());
        Eigen::AngleAxisd yawAngle(camera1.yaw, Eigen::Vector3d::UnitY());
        Eigen::AngleAxisd pitchAngle(camera1.pitch, Eigen::Vector3d::UnitX());

        Eigen::Quaternion<double> q = rollAngle * yawAngle * pitchAngle;

        Eigen::Matrix3d rotationMatrix = q.matrix();

        rotation(0, 0) = rotationMatrix(0, 0);
        rotation(0, 1) = rotationMatrix(0, 1);
        rotation(0, 2) = rotationMatrix(0, 2);
        rotation(0, 3) = 0;

        rotation(1, 0) = rotationMatrix(1, 0);
        rotation(1, 1) = rotationMatrix(1, 1);
        rotation(1, 2) = rotationMatrix(1, 2);
        rotation(1, 3) = 0;

        rotation(2, 0) = rotationMatrix(2, 0);
        rotation(2, 1) = rotationMatrix(2, 1);
        rotation(2, 2) = rotationMatrix(2, 2);
        rotation(2, 3) = 0;

        rotation(3, 0) = 0;
        rotation(3, 1) = 0;
        rotation(3, 2) = 0;
        rotation(3, 3) = 1;

        // Combine translation and rotation.
        cam = rotation * translation;

        return cam;
    }

    MatrixXf getPos3d(float x,float y,float z){
        MatrixXf Pos3D(4, 1); Pos3D.setZero();
        Pos3D(0, 0) = x;
        Pos3D(1, 0) = y;
        Pos3D(2, 0) = z;
        Pos3D(3, 0) = 1;

        return Pos3D;
    }

    MatrixXf getPos2d(float x, float y, float z){
        MatrixXf Pos3d(4, 1); Pos3d.setZero();
        Pos3d = getPos3d(x, y, z);
        MatrixXf projection_matrix(4, 4); projection_matrix.setZero();
        projection_matrix = getMper();
        MatrixXf camera_matrix(4, 4); camera_matrix.setZero();
        camera_matrix = getMcam();
        MatrixXf viewport_matrix(4, 4); viewport_matrix.setZero();
        viewport_matrix = getMvp();
        MatrixXf point_after_PM(3, 1); point_after_PM.setZero();
        point_after_PM = projection_matrix * (camera_matrix * Pos3d);
        point_after_PM = point_after_PM / point_after_PM(3); // Normalize by devide by point_after_PM[3]?
        MatrixXf point_after_VP(3, 1); point_after_VP.setZero();
        point_after_VP = viewport_matrix * point_after_PM;
        return point_after_VP;
    }

    species::position::TwoD point3d2point2d(species::position::ThreeD point3d){ // Naming scheme needs work
        MatrixXf Mpos2d(3, 1); Mpos2d.setZero();
        Mpos2d = getPos2d(point3d.x, point3d.y, point3d.z);
        species::position::TwoD pos2d;
        pos2d.x = Mpos2d(0, 0);
        pos2d.y = Mpos2d(1, 0);
        return pos2d;
    }

    species::Triangle polygon2triangle(species::Polygon polygon){
        species::Triangle triangle;
        triangle.a = point3d2point2d(polygon.a);
        triangle.b = point3d2point2d(polygon.b);
        triangle.c = point3d2point2d(polygon.c);
        triangle.color = polygon.color;
        return triangle;
    }
}

namespace draw{
    void triangle(species::Triangle triangle){
        std::vector< SDL_Vertex > triangle_info =
                {
                        { SDL_FPoint{ triangle.a.x, triangle.a.y }, triangle.color, SDL_FPoint{ 0 }, },
                        { SDL_FPoint{ triangle.b.x, triangle.b.y }, triangle.color, SDL_FPoint{ 0 }, },
                        { SDL_FPoint{ triangle.c.x, triangle.c.y }, triangle.color, SDL_FPoint{ 0 }, },
                };
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_RenderDrawLine(renderer, triangle.a.x, triangle.a.y, triangle.b.x, triangle.b.y);
        SDL_RenderDrawLine(renderer, triangle.b.x, triangle.b.y, triangle.c.x, triangle.c.y);
        SDL_RenderDrawLine(renderer, triangle.c.x, triangle.c.y, triangle.a.x, triangle.a.y);
        SDL_RenderGeometry(renderer, nullptr, triangle_info.data(), triangle_info.size(), nullptr, 0 );
    }
    void polygon(species::Polygon polygon){
        species::Triangle triangle1 = matrixShite::polygon2triangle(polygon);
        triangle(triangle1);
    }

    void screen() // Unlocked screen updater
    {
        SDL_SetRenderDrawColor(renderer, 0,0,0,255);
        SDL_RenderClear(renderer);
        polygon(polygon1);
        polygon(polygon2);
        SDL_RenderPresent(renderer);
    }
}


int main() {
    camera1.FovX = 1;
    camera1.FovY = 1;
//    temptriangle.a.x = 10;
//    temptriangle.a.y = 10;
//    temptriangle.b.x = 990;
//    temptriangle.b.y = 990;
//    temptriangle.c.x = 10;
//    temptriangle.c.y = 990;
//    temptriangle.color = SDL_Color{100, 100, 255, 255};

    std::cout << "Not really a twinstick but whatever!  " << std::endl;

    SDL_Init(SDL_INIT_VIDEO);
    SDL_CreateWindowAndRenderer(screen_width, screen_height, 0, &window, &renderer);
    SDL_RenderSetScale(renderer,1,1);
    SDL_SetWindowTitle(window, "Joystick thingy");
    camera1.x = 0;
    camera1.y = 1;
    camera1.z = 1.5;
    polygon1.a.x = -1;
    polygon1.a.y = -1;
    polygon1.a.z = -1;
    polygon1.b.x = 1;
    polygon1.b.y = -1;
    polygon1.b.z = -1;
    polygon1.c.x = 1;
    polygon1.c.y = 1;
    polygon1.c.z = -1;
    polygon1.color = SDL_Color{255, 255, 255, 255};
    polygon2 = polygon1;
    polygon2.c.y = -1;
    polygon2.c.z = 2;

    while (running) {
        SDL_Event event;
        draw::screen();
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    running = false;
                    break;
                case SDL_KEYDOWN:
                    if (event.key.keysym.sym ==SDLK_SPACE)paused = !paused;
                    if (event.key.keysym.sym ==SDLK_w)camera1.y -= 0.1;
                    if (event.key.keysym.sym ==SDLK_s)camera1.y += 0.1;
                    if (event.key.keysym.sym ==SDLK_d)camera1.x += 0.1;
                    if (event.key.keysym.sym ==SDLK_a)camera1.x -= 0.1;
                    if (event.key.keysym.sym ==SDLK_e)camera1.z += 0.1;
                    if (event.key.keysym.sym ==SDLK_q)camera1.z -= 0.1;
                    if (event.key.keysym.sym ==SDLK_o)camera1.roll += 0.05;
                    if (event.key.keysym.sym ==SDLK_u)camera1.roll -= 0.05;
                    if (event.key.keysym.sym ==SDLK_k)camera1.yaw += 0.05;
                    if (event.key.keysym.sym ==SDLK_i)camera1.yaw -= 0.05;
                    if (event.key.keysym.sym ==SDLK_l)camera1.pitch += 0.05;
                    if (event.key.keysym.sym ==SDLK_j)camera1.pitch -= 0.05;
                    if (event.key.keysym.sym ==SDLK_n)polygon1.a.z -= 0.1;
                    if (event.key.keysym.sym ==SDLK_m)polygon1.a.z += 0.1;
                    if (event.key.keysym.sym ==SDLK_y){
                        camera1.yaw = 0;
                        camera1.pitch = 0;
                        camera1.roll = 0;
                    };


                    break;
            }
            std::cout << "Roll: " << camera1.roll << "\nYaw: " << camera1.yaw << "\nPitch " << camera1.pitch << "\n";
        }
    }

    return 0;
}
