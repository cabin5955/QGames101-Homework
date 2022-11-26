#include "mainwindow.h"
#include <QCoreApplication>
#include <QPainter>
#include "Scene.hpp"
#include "Sphere.hpp"
#include "Triangle.hpp"
#include "Light.hpp"
#include "Renderer.hpp"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    resize(800, 600);
    setWindowTitle(tr("Raytracing Demo"));

    QString path =  QCoreApplication::applicationDirPath()+"binary.ppm";
    QByteArray ba = path.toLocal8Bit();
    const char *imgPath = ba.data();

    Scene scene(800, 600);
    auto sph1 = std::make_unique<Sphere>(Vector3f(-1, 0, -12), 2);
    sph1->materialType = DIFFUSE_AND_GLOSSY;
    sph1->diffuseColor = Vector3f(0.6, 0.7, 0.8);

    auto sph2 = std::make_unique<Sphere>(Vector3f(0.5, -0.5, -8), 1.5);
    sph2->ior = 1.5;
    sph2->materialType = REFLECTION_AND_REFRACTION;

    scene.Add(std::move(sph1));
    scene.Add(std::move(sph2));

    Vector3f verts[4] = {{-5,-3,-6}, {5,-3,-6}, {5,-3,-16}, {-5,-3,-16}};
    uint32_t vertIndex[6] = {0, 1, 3, 1, 2, 3};
    Vector2f st[4] = {{0, 0}, {1, 0}, {1, 1}, {0, 1}};
    auto mesh = std::make_unique<MeshTriangle>(verts, vertIndex, 2, st);
    mesh->materialType = DIFFUSE_AND_GLOSSY;

    scene.Add(std::move(mesh));
    scene.Add(std::make_unique<Light>(Vector3f(-20, 70, 20), 0.5));
    scene.Add(std::make_unique<Light>(Vector3f(30, 50, -12), 0.5));

    Renderer r;
    r.Render(scene, imgPath);

    canvas = new QImage(path);
}

void MainWindow::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.drawImage(0,0,*canvas);
}

MainWindow::~MainWindow()
{
}

