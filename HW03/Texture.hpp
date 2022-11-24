//
// Created by LEI XU on 4/27/19.
//

#ifndef RASTERIZER_TEXTURE_H
#define RASTERIZER_TEXTURE_H
#include "global.hpp"
#include <eigen/Eigen>
#include "QtGui/qimage.h"

class Texture{
private:
    //cv::Mat image_data;
    QImage img_data ;

public:
    Texture(const std::string& name)
    {
        //image_data = cv::imread(name);
        QString file = QString::fromStdString(name);
        img_data = QImage(file);
        //img_data = img.mirrored(false,true);
        //cv::cvtColor(image_data, image_data, cv::COLOR_RGB2BGR);
        width = img_data.width();
        height = img_data.height();
    }

    int width, height;

    Eigen::Vector3f getColor(float u, float v)
    {
        u = std::fmin(1, std::fmax(u, 0));
        v = std::fmin(1, std::fmax(v, 0));
        auto u_img = u * width;
        auto v_img = (1 - v) * height;
        //auto color = image_data.at<cv::Vec3b>(v_img, u_img);
        QColor color( img_data.pixel( u_img, v_img ) );
        return Eigen::Vector3f(color.red(), color.green(), color.blue());
    }

};
#endif //RASTERIZER_TEXTURE_H
