#include "CImg.h"
#include <cmath>
#include <algorithm>

using namespace cimg_library;
static const double EPS = 1e-5;
static const double pi = 3.1415926535;

typedef struct Point {
    double x, y;
    Point(double x, double y) : x(x), y(y) {}
    Point() : x(0), y(0) {}
} point;

// 用于将图片缩放size倍
CImg<unsigned char> myresize(CImg<unsigned char>& img, double size) {
    if (size == 1) return img;
    unsigned int nw = img.width() * size, nh = img.height() * size;
    CImg<unsigned char> re(nw, nh, 1, 1, 0);
    cimg_forXY(re, x, y) {

        // // 双线性插值
        // double ox = x / size, oy = y / size;
        // unsigned int ix0 = (unsigned int)ox, iy0 = (unsigned int)oy;
        // unsigned int ix1 = (ix0 + 1 >= img.width() ? ix0 : ix0 + 1), \
        //     iy1 = (iy0 + 1 >= img.height() ? iy0 : iy0 + 1);
        // double px = ox - ix0, py = oy - iy0;
        // double color = img(ix0, iy0) * (1- px) * (1 - py) + \
        //                img(ix1, iy0) * px * (1- py) + \
        //                img(ix0, iy1) * (1 - px) * py + \
        //                img(ix1, iy1) * px * py;

        // re(x, y) = std::floor(color + 0.5);

        double ox = x / size, oy = y / size, oxx = (x + 1) / size, oyy = (y + 1) / size;
        unsigned int xlu = std::floor(ox), ylu = std::floor(oy), \
            xrd = std::ceil(oxx), yrd = std::ceil(oyy);
        double csum = 0.0, psum = 0.0, p = 1.0;
        for (unsigned int i = xlu; i < xrd; i++) {
            for (unsigned int j = ylu; j < yrd; j++) {
                p = 1.0;
                if (x < ox) p *= (1 - (ox - x));
                else if (x > oxx) p *= (1 - (x - oxx));
                if (y < ox) p *= (1 - (oy - y));
                else if (y > oyy) p *= (1 - (y - oyy));
                csum += img(i, j) * p;
                psum += p;
            }
        }
        re(x, y) = (unsigned char)(csum / psum);
    }
    return re;
}

// 用于将图片顺时针旋转90、180、270
CImg<unsigned char> mytran(CImg<unsigned char>& img, unsigned int t) {
    switch(t) {
        case 1: {
            // 旋转90
            CImg<unsigned char> re(img.height(), img.width());
            cimg_forXY(re, x, y) {
                re(x, y) = img(y, img.width() - x);
            }
            return re;
            break;
        }
        case 2: {
            // 旋转180
            CImg<unsigned char> re(img.width(), img.height());
            cimg_forXY(re, x, y) {
                re(x, y) = img(img.width() - x, img.width() - y);
            }
            return re;
            break;
        }
        case 3: {
            // 旋转270
            CImg<unsigned char> re(img.height(), img.width());
            cimg_forXY(re, x, y) {
                re(x, y) = img(img.height() - y, x);
            }
            return re;
            break;
        }
        default: {
            // 不旋转
            CImg<unsigned char> re(img);
            return re;
        }
    }
}

// 用于旋转图片，angle为一平面角
CImg<unsigned char> myrotate(CImg<unsigned char>& img, double angle) {
    unsigned int angle2 = (unsigned int)angle % 360;
    CImg<unsigned char> tranedImg = mytran(img, angle2 / 90);
    angle2 %= 90;
    if (angle2 == 0) return tranedImg;

    angle = angle2 + (angle - (unsigned int)angle);
    angle = angle / 180 * pi;

    unsigned int nw = tranedImg.width() * std::sin(angle) + tranedImg.height() * std::cos(angle), \
        nh = tranedImg.width() * std::cos(angle) + tranedImg.height() * std::sin(angle);

    CImg<unsigned char> re(nw, nh, 1, 1, 0);
    double transMidx = tranedImg.width() / 2.0, transMidy = tranedImg.height() / 2.0, \
           resMidX = re.width() / 2.0, resMidy = re.height() / 2.0;

    cimg_forXY(re, x, y) {
        // 遍历旋转后图片的像素点，计算出对应的原图中的位置，若无对应则跳过
        // 先把坐标系转换为以图像中心为原点的极坐标系，计算后再将极坐标系转换为直角坐标
        double ox = transMidx + std::cos(angle) * (x - resMidX) - std::sin(angle) * (resMidy - y), \
               oy = transMidy - std::sin(angle) * (x - resMidX) - std::cos(angle) * (resMidy - y);

        if (ox < EPS || oy < EPS || ox >= tranedImg.width() ||  oy >= tranedImg.height()) {
            continue;
        }
        
        // 双线性插值
        unsigned int ix0 = (unsigned int)ox, iy0 = (unsigned int)oy;
        unsigned int ix1 = (ix0 + 1 >= tranedImg.width() ? ix0 : ix0 + 1), \
            iy1 = (iy0 + 1 >= tranedImg.height() ? iy0 : iy0 + 1);
        double px = ox - ix0, py = oy - iy0;
        double color = tranedImg(ix0, iy0) * (1- px) * (1 - py) + \
                       tranedImg(ix1, iy0) * px * (1- py) + \
                       tranedImg(ix0, iy1) * (1 - px) * py + \
                       tranedImg(ix1, iy1) * px * py;

        re(x, y) = std::floor(color + 0.5);
    }
    return re;
}

// 返回点p0到p1、p2所成直线的距离
double dist(point p0, point p1, point p2) {
    return std::fabs((p2.y - p1.y) * p0.x + (p1.x - p2.x) * p0.y + \
            p2.x * p1.y - p1.x * p2.y) / \
            std::sqrt(std::pow(p2.y - p1.y, 2) + std::pow(p1.x - p2.x, 2));
}

// 返回p0、p1间的距离
double dist(point p0, point p1) {
    return std::sqrt(std::pow(p0.y - p1.y, 2) + std::pow(p0.x - p1.x, 2));
}

// 画出由p1 p2 p3三点确定的矩形（平行四边形），若不是矩形会有提示，bound为边宽
void drawrectangle(CImg<unsigned char>& img, point p1, point p2, point p3, unsigned int bound) {
    double d12 = dist(p1, p2), d13 = dist(p1, p3), d23 = dist(p2, p3);

    // 确保p2、p3在矩形中是相对的两点，同时确定是否是矩形
    if (fabs(d12 * d12 + d13 * d13 - d23 * d23) >= EPS) {
        if (fabs(d13 * d13 + d23 * d23 - d12 * d12) < EPS) {
            std::swap(p1, p3);
            std::swap(d12, d23);
        } else if (fabs(d12 * d12 + d23 * d23 - d13 * d13) < EPS) {
            std::swap(p1, p2);
            std::swap(d13, d23);
        } else {
            std::cout << "WARRANTY: Not a rectangle.\n";
        }
    }

    // 类似画直角三角形，根据对称性画出矩形（平行四边形）
    unsigned int mx = p2.x + p3.x, my = p2.y + p3.y;
    cimg_forXY(img, x, y) {
        point p0(x, y);
        if (  (dist(p0, p1, p2) < bound / 2 && dist(p0, p1) + dist(p0, p2) < d12 + bound)
           || (dist(p0, p1, p3) < bound / 2 && dist(p0, p1) + dist(p0, p3) < d13 + bound)) {
            img(p0.x, p0.y) = 0;
            img(mx - p0.x, my - p0.y) = 0;
        }
    }
}

// 画出由p1 p2 p3三点确定的三角形，bound为边宽
void drawreiangle(CImg<unsigned char>& img, point p1, point p2, point p3, unsigned int bound) {
    double d12 = dist(p1, p2), d13 = dist(p1, p3), d23 = dist(p2, p3);
    cimg_forXY(img, x, y) {
        point p0(x, y);
        if (  (dist(p0, p1, p2) < bound / 2 && dist(p0, p1) + dist(p0, p2) < d12 + bound)
           || (dist(p0, p1, p3) < bound / 2 && dist(p0, p1) + dist(p0, p3) < d13 + bound)
           || (dist(p0, p2, p3) < bound / 2 && dist(p0, p2) + dist(p0, p3) < d23 + bound))
            img(x, y) = 0;
    }
}

// 画出以p1为原点，r为半径的圆，bound为边宽
void drawcicle(CImg<unsigned char>& img, point p1, unsigned int r, unsigned int bound) {
    cimg_forXY(img, x, y) {
        point p0(x, y);
        if (fabs(dist(p0, p1) - r) < bound / 2) img(x, y) = 0;
    }
}
