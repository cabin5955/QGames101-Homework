#include "mainwindow.h"
#include <QPainter>
#include <eigen/Geometry>
#include <iostream>
#include "global.hpp"
#include "Shader.hpp"
#include "Texture.hpp"
#include "OBJ_Loader.h"
#include "Triangle.hpp"
#include "rasterizer.hpp"
#include <eigen/Eigen>
#include <cmath>

using namespace Eigen;

#define WIDTH 700
#define HEIGHT 700

Eigen::Matrix4f get_view_matrix(Eigen::Vector3f eye_pos)
{
    Eigen::Matrix4f view = Eigen::Matrix4f::Identity();

    Eigen::Matrix4f translate;
    translate << 1,0,0,-eye_pos[0],
                 0,1,0,-eye_pos[1],
                 0,0,1,-eye_pos[2],
                 0,0,0,1;

    view = translate*view;

    return view;
}

Eigen::Matrix4f get_model_matrix(float angle)
{
    Eigen::Matrix4f rotation;
    angle = angle * MY_PI / 180.f;
    rotation << cos(angle), 0, sin(angle), 0,
                0, 1, 0, 0,
                -sin(angle), 0, cos(angle), 0,
                0, 0, 0, 1;

    Eigen::Matrix4f scale;
    scale << 2.5, 0, 0, 0,
              0, 2.5, 0, 0,
              0, 0, 2.5, 0,
              0, 0, 0, 1;

    Eigen::Matrix4f translate;
    translate << 1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1;

    return translate * rotation * scale;
}

Eigen::Matrix4f get_projection_matrix(float eye_fov, float aspect_ratio, float zNear, float zFar)
{
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

Eigen::Vector3f vertex_shader(const vertex_shader_payload& payload)
{
    return payload.position;
}

Eigen::Vector3f normal_fragment_shader(const fragment_shader_payload& payload)
{
    Eigen::Vector3f return_color = (payload.normal.head<3>().normalized() + Eigen::Vector3f(1.0f, 1.0f, 1.0f)) / 2.f;
    Eigen::Vector3f result;
    result << return_color.x() * 255, return_color.y() * 255, return_color.z() * 255;
    return result;
}

static Eigen::Vector3f reflect(const Eigen::Vector3f& vec, const Eigen::Vector3f& axis)
{
    auto costheta = vec.dot(axis);
    return (2 * costheta * axis - vec).normalized();
}

struct light
{
    Eigen::Vector3f position;
    Eigen::Vector3f intensity;
};

Eigen::Vector3f texture_fragment_shader(const fragment_shader_payload& payload)
{
    Eigen::Vector3f return_color = {0, 0, 0};
    if (payload.texture)
    {
        // TODO: Get the texture value at the texture coordinates of the current fragment
        return_color = payload.texture->getColor(payload.tex_coords.x(),payload.tex_coords.y());
    }
    Eigen::Vector3f texture_color;
    texture_color << return_color.x(), return_color.y(), return_color.z();

    Eigen::Vector3f ka = Eigen::Vector3f(0.005, 0.005, 0.005);
    Eigen::Vector3f kd = texture_color / 255.f;
    Eigen::Vector3f ks = Eigen::Vector3f(0.7937, 0.7937, 0.7937);

    auto l1 = light{{20, 20, 20}, {500, 500, 500}};
    auto l2 = light{{-20, 20, 0}, {500, 500, 500}};

    std::vector<light> lights = {l1, l2};
    Eigen::Vector3f amb_light_intensity{10, 10, 10};
    Eigen::Vector3f eye_pos{0, 0, 10};

    float p = 150;

    Eigen::Vector3f color = texture_color;
    Eigen::Vector3f point = payload.view_pos;
    Eigen::Vector3f normal = payload.normal;

    Eigen::Vector3f result_color = {0, 0, 0};

    // La = ka * Ia
    auto ambient = ka.cwiseProduct(amb_light_intensity);
    result_color += ambient;

    for (auto& light : lights)
    {
        // TODO: For each light source in the code, calculate what the *ambient*, *diffuse*, and *specular*
        // components are. Then, accumulate that result on the *result_color* object.

        float distance_square =
                pow((light.position.x() - point.x()), 2) +
                pow((light.position.y() - point.y()), 2) +
                pow((light.position.z() - point.z()), 2);
        auto n = normal.normalized();

        // Ld = kd * (I / r ^ 2) * max(0, n.dot(l))
        auto light_direction = (light.position - point).normalized();
        auto diffuse = kd.cwiseProduct((light.intensity / distance_square)) * std::max(0.f, n.dot(light_direction));

        // Ls = ks * (I / r ^ 2) * max(0, n.dot(h)) ^ p
        auto v = (eye_pos - point).normalized();
        auto h = (v + light_direction).normalized();
        auto specular = ks.cwiseProduct((light.intensity / distance_square)) * pow(std::max(0.f, n.dot(h)), p);

        result_color += (diffuse + specular);
    }

    return result_color * 255.f;
}

Eigen::Vector3f phong_fragment_shader(const fragment_shader_payload& payload)
{
    Eigen::Vector3f ka = Eigen::Vector3f(0.005, 0.005, 0.005);
    Eigen::Vector3f kd = payload.color / 255.f;
    Eigen::Vector3f ks = Eigen::Vector3f(0.7937, 0.7937, 0.7937);

    auto l1 = light{{20, 20, 20}, {500, 500, 500}};
    auto l2 = light{{-20, 20, 0}, {500, 500, 500}};

    std::vector<light> lights = {l1, l2};
    Eigen::Vector3f amb_light_intensity{10, 10, 10};
    Eigen::Vector3f eye_pos{0, 0, 10};

    float p = 150;

    Eigen::Vector3f color = payload.color;
    Eigen::Vector3f point = payload.view_pos;
    Eigen::Vector3f normal = payload.normal;

    Eigen::Vector3f result_color = {0, 0, 0};

    // La = ka * Ia
    auto ambient = ka.cwiseProduct(amb_light_intensity);
    result_color += ambient;

    for (auto& light : lights)
    {
        // TODO: For each light source in the code, calculate what the *ambient*, *diffuse*, and *specular*
        // components are. Then, accumulate that result on the *result_color* object.
        float distance_square =
                pow((light.position.x() - point.x()), 2) +
                pow((light.position.y() - point.y()), 2) +
                pow((light.position.z() - point.z()), 2);
        auto n = normal.normalized();

        // Ld = kd * (I / r ^ 2) * max(0, n.dot(l))
        auto light_direction = (light.position - point).normalized();
        auto diffuse = kd.cwiseProduct((light.intensity / distance_square)) * std::max(0.f, n.dot(light_direction));

        // Ls = ks * (I / r ^ 2) * max(0, r.dot(v)) ^ p
        auto v = (eye_pos - point).normalized();
        auto r = reflect(light_direction,n);
        auto specular = ks.cwiseProduct((light.intensity / distance_square)) * pow(std::max(0.f, r.dot(v)), p);

        result_color += (diffuse + specular);
    }

    return result_color * 255.f;
}



Eigen::Vector3f displacement_fragment_shader(const fragment_shader_payload& payload)
{

    Eigen::Vector3f ka = Eigen::Vector3f(0.005, 0.005, 0.005);
    Eigen::Vector3f kd = payload.color;
    Eigen::Vector3f ks = Eigen::Vector3f(0.7937, 0.7937, 0.7937);

    auto l1 = light{{20, 20, 20}, {500, 500, 500}};
    auto l2 = light{{-20, 20, 0}, {500, 500, 500}};

    std::vector<light> lights = {l1, l2};
    Eigen::Vector3f amb_light_intensity{10, 10, 10};
    Eigen::Vector3f eye_pos{0, 0, 10};

    float p = 150;

    Eigen::Vector3f color = payload.color;
    Eigen::Vector3f point = payload.view_pos;
    Eigen::Vector3f normal = payload.normal;

    float kh = 0.2, kn = 0.1;

    // TODO: Implement displacement mapping here
    // Let n = normal = (x, y, z)
    // Vector t = (x*y/sqrt(x*x+z*z),sqrt(x*x+z*z),z*y/sqrt(x*x+z*z))
    // Vector b = n cross product t
    // Matrix TBN = [t b n]
    // dU = kh * kn * (h(u+1/w,v)-h(u,v))
    // dV = kh * kn * (h(u,v+1/h)-h(u,v))
    // Vector ln = (-dU, -dV, 1)
    // Position p = p + kn * n * h(u,v)
    // Normal n = normalize(TBN * ln)

    float x = normal.x();
    float y = normal.y();
    float z = normal.z();

    Eigen::Vector3f t = Eigen::Vector3f(x*y/sqrt(x*x+z*z),sqrt(x*x+z*z),z*y/sqrt(x*x+z*z));
    Eigen::Vector3f b = normal.cross(t);
    Eigen::Matrix3f TBN;
    TBN << t.x(), b.x(), normal.x(),
           t.y(), b.y(), normal.y(),
           t.z(), b.z(), normal.z();

    float u = payload.tex_coords.x();
    float v = payload.tex_coords.y();
    float w = payload.texture->width;
    float h = payload.texture->height;

    float dU = kh * kn * (payload.texture->getColor(u+1.0/w,v).norm()-payload.texture->getColor(u,v).norm());
    float dV = kh * kn * (payload.texture->getColor(u,v+1.0/h).norm()-payload.texture->getColor(u,v).norm());
    Eigen::Vector3f ln = Eigen::Vector3f(-dU, -dV, 1);
    point += kn * normal * payload.texture->getColor(u,v).norm();
    normal = (TBN * ln).normalized();

    Eigen::Vector3f result_color = {0, 0, 0};
    // La = ka * Ia
    auto ambient = ka.cwiseProduct(amb_light_intensity);
    result_color += ambient;

    for (auto& light : lights)
    {
        // TODO: For each light source in the code, calculate what the *ambient*, *diffuse*, and *specular*
        // components are. Then, accumulate that result on the *result_color* object.
        float distance_square =
                pow((light.position.x() - point.x()), 2) +
                pow((light.position.y() - point.y()), 2) +
                pow((light.position.z() - point.z()), 2);
        auto n = normal;

        // Ld = kd * (I / r ^ 2) * max(0, n.dot(l))
        auto light_direction = (light.position - point).normalized();
        auto diffuse = kd.cwiseProduct((light.intensity / distance_square)) * std::max(0.f, n.dot(light_direction));

        // Ls = ks * (I / r ^ 2) * max(0, r.dot(v)) ^ p
        auto v = (eye_pos - point).normalized();
        auto r = reflect(light_direction,n);
        auto specular = ks.cwiseProduct((light.intensity / distance_square)) * pow(std::max(0.f, r.dot(v)), p);

        result_color += (diffuse + specular);
    }

    return result_color * 255.f;
}


Eigen::Vector3f bump_fragment_shader(const fragment_shader_payload& payload)
{

    Eigen::Vector3f ka = Eigen::Vector3f(0.005, 0.005, 0.005);
    Eigen::Vector3f kd = payload.color;
    Eigen::Vector3f ks = Eigen::Vector3f(0.7937, 0.7937, 0.7937);

    auto l1 = light{{20, 20, 20}, {500, 500, 500}};
    auto l2 = light{{-20, 20, 0}, {500, 500, 500}};

    std::vector<light> lights = {l1, l2};
    Eigen::Vector3f amb_light_intensity{10, 10, 10};
    Eigen::Vector3f eye_pos{0, 0, 10};

    float p = 150;

    Eigen::Vector3f color = payload.color;
    Eigen::Vector3f point = payload.view_pos;
    Eigen::Vector3f normal = payload.normal;

    float kh = 0.2, kn = 0.1;

    // TODO: Implement bump mapping here
    // Let n = normal = (x, y, z)
    // Vector t = (x*y/sqrt(x*x+z*z),sqrt(x*x+z*z),z*y/sqrt(x*x+z*z))
    // Vector b = n cross product t
    // Matrix TBN = [t b n]
    // dU = kh * kn * (h(u+1/w,v)-h(u,v))
    // dV = kh * kn * (h(u,v+1/h)-h(u,v))
    // Vector ln = (-dU, -dV, 1)
    // Normal n = normalize(TBN * ln)

    float x = normal.x();
    float y = normal.y();
    float z = normal.z();

    Eigen::Vector3f t = Eigen::Vector3f(x*y/sqrt(x*x+z*z),sqrt(x*x+z*z),z*y/sqrt(x*x+z*z));
    Eigen::Vector3f b = normal.cross(t);
    Eigen::Matrix3f TBN;
    TBN << t.x(), b.x(), normal.x(),
           t.y(), b.y(), normal.y(),
           t.z(), b.z(), normal.z();

    float u = payload.tex_coords.x();
    float v = payload.tex_coords.y();
    float w = payload.texture->width;
    float h = payload.texture->height;

    float dU = kh * kn * (payload.texture->getColor(u+1.0/w,v).norm()-payload.texture->getColor(u,v).norm());
    float dV = kh * kn * (payload.texture->getColor(u,v+1.0/h).norm()-payload.texture->getColor(u,v).norm());
    Eigen::Vector3f ln = Eigen::Vector3f(-dU, -dV, 1);
    normal = (TBN * ln).normalized();

    Eigen::Vector3f result_color = {0, 0, 0};
    result_color = normal;

    return result_color * 255.f;
}

void test_simple_triangle(std::vector<Triangle*> &TriangleList)
{
    //test simple triangle
    Eigen::Vector4f vertices[] = {{0.5f,  0.5f, 0.0f, 1.0f},
                                  {0.5f, -0.5f, 0.0f, 1.0f},
                                  {-0.5f, -0.5f, 0.0f, 1.0f},
                                  {-0.5f,  0.5f, 0.0f, 1.0f}};

    Triangle* t1 = new Triangle();
    t1->setVertex(0, vertices[0]);
    t1->setVertex(1, vertices[1]);
    t1->setVertex(2, vertices[3]);
    t1->setTexCoord(0,Vector2f(1.0f, 1.0f));
    t1->setTexCoord(1,Vector2f(1.0f, 0.0f));
    t1->setTexCoord(2,Vector2f(0.0f, 1.0f));
    t1->setNormal(0,Vector3f(0.0f,0.0f,1.0f));
    t1->setNormal(1,Vector3f(0.0f,0.0f,1.0f));
    t1->setNormal(2,Vector3f(0.0f,0.0f,1.0f));
    TriangleList.push_back(t1);

    Triangle* t2 = new Triangle();
    t2->setVertex(0, vertices[1]);
    t2->setVertex(1, vertices[2]);
    t2->setVertex(2, vertices[3]);
    t2->setTexCoord(0,Vector2f(1.0f, 0.0f));
    t2->setTexCoord(1,Vector2f(0.0f, 0.0f));
    t2->setTexCoord(2,Vector2f(0.0f, 1.0f));
    t2->setNormal(0,Vector3f(0.0f,0.0f,1.0f));
    t2->setNormal(1,Vector3f(0.0f,0.0f,1.0f));
    t2->setNormal(2,Vector3f(0.0f,0.0f,1.0f));
    TriangleList.push_back(t2);
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    resize(WIDTH, HEIGHT);
    setWindowTitle(tr("Rasterize Demo"));

    std::vector<Triangle*> TriangleList;
    float angle = 140.0;

    std::string filename = "output.png";
    objl::Loader Loader;
    std::string obj_path = ":/models/models/spot/";

    // Load .obj File
    //bool loadout = Loader.LoadFile(":/models/models/cube/cube.obj");
    bool loadout = Loader.LoadFile(":/models/models/spot/spot_triangulated_good.obj");
    for(auto mesh:Loader.LoadedMeshes)
    {
        for(int i=0;i<mesh.Vertices.size();i+=3)
        {
            Triangle* t = new Triangle();
            for(int j=0;j<3;j++)
            {
                t->setVertex(j,Vector4f(mesh.Vertices[i+j].Position.X,mesh.Vertices[i+j].Position.Y,mesh.Vertices[i+j].Position.Z,1.0));
                t->setNormal(j,Vector3f(mesh.Vertices[i+j].Normal.X,mesh.Vertices[i+j].Normal.Y,mesh.Vertices[i+j].Normal.Z));
                t->setTexCoord(j,Vector2f(mesh.Vertices[i+j].TextureCoordinate.X, mesh.Vertices[i+j].TextureCoordinate.Y));
            }
            TriangleList.push_back(t);
        }
    }

    rst::rasterizer r(700, 700);

    auto texture_path = "hmap.jpg";//"spot_texture.png";
    r.set_texture(Texture(obj_path + texture_path));

    std::function<Eigen::Vector3f(fragment_shader_payload)> active_shader = displacement_fragment_shader;

    Eigen::Vector3f eye_pos = {0,0,10};
    r.set_vertex_shader(vertex_shader);
    r.set_fragment_shader(active_shader);
    r.clear(rst::Buffers::Color | rst::Buffers::Depth);
    r.set_model(get_model_matrix(angle));
    r.set_view(get_view_matrix(eye_pos));
    r.set_projection(get_projection_matrix(45.0, 1, 0.1, 50));

    r.draw(TriangleList);

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
