#include "mainwindow.h"
#include <QPainter>
#include <QMouseEvent>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    resize(800, 600);
    setWindowTitle(tr("Bezier Demo"));
}

MainWindow::~MainWindow()
{
}

void MainWindow::naive_bezier(const QVector<QPointF> &points)
{
    auto &p_0 = points[0];
    auto &p_1 = points[1];
    auto &p_2 = points[2];
    auto &p_3 = points[3];

    QPainter painter(this);
    QPen ctrlPen1(QColor(0, 0, 255));
    ctrlPen1.setWidth(2);
    painter.setPen(ctrlPen1);
    for (double t = 0.0; t <= 1.0; t += 0.001)
    {
        auto point = std::pow(1 - t, 3) * p_0 + 3 * t * std::pow(1 - t, 2) * p_1 +
                 3 * std::pow(t, 2) * (1 - t) * p_2 + std::pow(t, 3) * p_3;
        painter.drawPoint(point);
    }
}

QPointF MainWindow::recursive_bezier(const QVector<QPointF> &control_points, int n, int k, double t)
{
    // TODO: Implement de Casteljau's algorithm
    if(n==1)
    {
        return (1-t)*control_points[k] + t*control_points[k+1];
    }
    else
    {
        return (1-t) * recursive_bezier(control_points, n-1 , k, t) +
               t * recursive_bezier(control_points, n-1 , k+1, t);
    }
}

void MainWindow::bezier(const QVector<QPointF> &control_points)
{
    // TODO: Iterate through all t = 0 to t = 1 with small steps, and call de Casteljau's
    // recursive Bezier algorithm.
    QPainter painter(this);
    QPen ctrlPen1(QColor(0, 0, 255));
    ctrlPen1.setWidth(2);
    painter.setPen(ctrlPen1);

    int n = control_points.size()-1;
    int k = 0;
    for (double t = 0.0; t <= 1.0; t += 0.001)
    {
        QPointF point = recursive_bezier(control_points, n, k, t);
        painter.drawPoint(point);
    }
}

void MainWindow::paintEvent(QPaintEvent *)
{

    QPainter painter(this);
    QPen ctrlPen1(QColor(255, 0, 0));
    ctrlPen1.setWidth(5);
    painter.setPen(ctrlPen1);

    for (auto &point : m_ctrlPoints)
    {
        painter.drawPoint(point);
    }

    if (m_ctrlPoints.size() == 4)
    {
       //naive_bezier(m_ctrlPoints);
        bezier(m_ctrlPoints);
    }
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    // 单击鼠标左键获取控制点
    if (event->buttons() == Qt::LeftButton){
        m_ctrlPoints.push_back(event->pos());
    }
    // 单击鼠标右键清空控制点
    else if (event->buttons() == Qt::RightButton) {
        m_ctrlPoints.clear();
    }
    update();
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{

}

void MainWindow::mouseDoubleClickEvent(QMouseEvent *event)
{

}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{

}
