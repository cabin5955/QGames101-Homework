#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseReleaseEvent(QMouseEvent *event) override;
    virtual void mouseDoubleClickEvent(QMouseEvent *event) override;
    virtual void mouseMoveEvent(QMouseEvent *event) override;
    virtual void paintEvent(QPaintEvent *) override;

private:
    void naive_bezier(const QVector<QPointF> &points);
    QPointF recursive_bezier(const QVector<QPointF> &control_points, int n, int k, double t);
    void bezier(const QVector<QPointF> &control_points);

    QImage *canvas;
    QVector<QPointF>    m_ctrlPoints;       // 控制点
    QVector<QPointF>    m_curvePoints;      // 曲线上的点
};
#endif // MAINWINDOW_H
