#include "mainwindow.h"
#include <QPainter>
#include <eigen/Geometry>
#include <iostream>
#include "Triangle.hpp"
#include "rasterizer.hpp"
#include <eigen/Eigen>
#include <cmath>

using namespace Eigen;

#define WIDTH 700
#define HEIGHT 700

constexpr double MY_PI = 3.1415926;

Eigen::Matrix4f get_view_matrix(Eigen::Vector3f eye_pos)
{
    Eigen::Matrix4f view = Eigen::Matrix4f::Identity();

    Eigen::Matrix4f translate;
    translate << 1, 0, 0, -eye_pos[0],
                 0, 1, 0, -eye_pos[1],
                 0, 0, 1, -eye_pos[2],
                 0, 0, 0, 1;

    view = translate * view;

    return view;
}

Eigen::Matrix4f get_model_matrix(float rotation_angle)
{
    Eigen::Matrix4f model = Eigen::Matrix4f::Identity();

    // TODO: Implement this function
    // Create the model matrix for rotating the triangle around the Z axis.
    // Then return it.

    float degree = (rotation_angle/2.0f) * MY_PI /180.0f;
    model(0, 0) = cos(degree);
    model(1, 1) = cos(degree);
    model(0, 1) = -sin(degree);
    model(1, 0) = sin(degree);

    return model;
}

Eigen::Matrix4f get_projection_matrix(float eye_fov, float aspect_ratio,
                                      float zNear, float zFar)
{
    // Students will implement this function

    Eigen::Matrix4f projection = Eigen::Matrix4f::Identity();

    // TODO: Implement this function
    // Create the projection matrix for the given parameters.
    // Then return it.

    float degree = (eye_fov/2.0f) * MY_PI /180.0f;
    float cot = 1.0f/tan(degree);

    projection(0, 0) = cot/aspect_ratio;
    projection(1, 1) = cot;
    projection(2, 2) = - (zFar + zNear) / (zFar - zNear);
    projection(3, 2) = -1;
    projection(2, 3) = - (2 * zFar * zNear) / (zFar - zNear);
    projection(3, 3) = 0;

    return projection;
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    resize(WIDTH, HEIGHT);
    setWindowTitle(tr("Rasterize Demo"));

//    Matrix2d a;
//    a << 1, 2, 3, 4;
//    MatrixXd b(2,2);
//    b << 2, 3, 1, 4;
//    std::cout << "a + b =\n" << a + b << std::endl;

    float angle = 30;
    rst::rasterizer r(WIDTH, HEIGHT);

    Eigen::Vector3f eye_pos = {0, 0, 5};

    std::vector<Eigen::Vector3f> pos
                {
                        {2, 0, -2},
                        {0, 2, -2},
                        {-2, 0, -2},
                        {3.5, -1, -5},
                        {2.5, 1.5, -5},
                        {-1, 0.5, -5}
                };

    std::vector<Eigen::Vector3i> ind
            {
                    {0, 1, 2},
                    {3, 4, 5}
            };

    std::vector<Eigen::Vector3f> cols
            {
                    {217.0, 238.0, 185.0},
                    {217.0, 238.0, 185.0},
                    {217.0, 238.0, 185.0},
                    {185.0, 217.0, 238.0},
                    {185.0, 217.0, 238.0},
                    {185.0, 217.0, 238.0}
            };

    auto pos_id = r.load_positions(pos);
    auto ind_id = r.load_indices(ind);
    auto col_id = r.load_colors(cols);

    Eigen::Matrix4f projection = get_projection_matrix(45, 1, 0.1, 50);
    Eigen::Matrix4f model = get_model_matrix(angle);

    r.clear(rst::Buffers::Color | rst::Buffers::Depth);
    r.set_model(model);
    r.set_view(get_view_matrix(eye_pos));
    r.set_projection(projection);

    r.draw(pos_id, ind_id, col_id, rst::Primitive::Triangle);

    unsigned char colorBuff[WIDTH * HEIGHT * 4];
    auto frameBuff = r.frame_buffer();

    for(int i=0;i<HEIGHT;i++)
    {
        for(int j=0;j<WIDTH;j++)
        {
            colorBuff[(i*WIDTH+j)*4+0] = frameBuff[i*WIDTH+j].x();
            colorBuff[(i*WIDTH+j)*4+1] = frameBuff[i*WIDTH+j].y();
            colorBuff[(i*WIDTH+j)*4+2] = frameBuff[i*WIDTH+j].z();
            colorBuff[(i*WIDTH+j)*4+3] = 255;
        }
    }

    canvas = new QImage(colorBuff, WIDTH, HEIGHT, QImage::Format_RGBA8888);

}

MainWindow::~MainWindow()
{
}

void MainWindow::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.drawImage(0,0,*canvas);
}

