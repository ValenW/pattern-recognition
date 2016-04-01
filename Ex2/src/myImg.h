#include "CImg.h"
#include <cmath>
#include <algorithm>
#include <list>
#include <vector>
#include <ctime>

using namespace cimg_library;
using namespace std;
static const double EPS = 1e-5;
const double alpha = 25, sigma = 5;
const int bound = 7;
const double thetamax = 2 * cimg::PI;

typedef struct Point {
    double x, y;
    Point(double x, double y) : x(x), y(y) {}
    Point() : x(0), y(0) {}
    bool operator ==(const Point& o) {
        return x == o.x && y == o.y;
    }
} point;

double calTimeCost() {
    static clock_t start = clock();
    double re = ((double)clock() - start) / CLOCKS_PER_SEC;
    start = clock();
    return re;
}

// 判断两以theta1 2的直线是否平行
bool parallel(double const &theta1, double const &theta2) {
    double diff = fabs(theta1 - theta2);
    return diff < cimg::PI / 18 || fabs(diff - cimg::PI) < cimg::PI / 18;
}

// 判断是否能组成封闭的left边形
bool calcir(int from, int now, int end, int n, int left, int l[][32] , bool *v) {
    if (left == 1) {
        if (l[now][end] > 0) { v[l[now][end] - 1] = true; return true; }
        return false;
    }
    for (int i = 0; i < n; i++) {
        if (l[now][i] > 0 && i != from && i != end) {
            if (calcir(now, i, end, n, left - 1, l, v)) {
                v[l[now][i] - 1] = true;
                return true;
            }
        }
    }
    return false;
}

// 返回经过点cx, cy，方向投影为gx, gy直线的极坐标系参数
point getHough(double cx, double cy, double gx, double gy) {
    double
        theta = atan2(gy, gx),
        rho   = sqrt(cx * cx + cy * cy) * cos(atan2(cy, cx) - theta);
    if (rho < 0) { rho = -rho; theta += cimg::PI; }
    theta = cimg::mod(theta, thetamax);
    return point(theta, rho);
}

// 返回img的huogh投票结果
CImg<> getVote(CImg<unsigned char> const &img) {
    CImg<> vote(500, 400, 1, 1, 0);
    double rhomax = sqrt(img.height() * img.height() + img.width() * img.width()) / 2.0;

    // 取得img每个像素点的梯度，默认使用的是旋转不变算子，并进行高斯平滑处理
    CImgList<> grad = img.get_gradient();
    cimglist_for(grad, l) grad[l].blur((float)alpha);

    // 投票，每个点的梯度越大，拥有票数越多:sqrt(gx * gx + gy * gy)
    cimg_forXY(img, x, y) {
        double
            cx = (double)x - img.width() / 2,
            cy = (double)y - img.height() / 2,
            gx = grad[0](x, y),
            gy = grad[1](x, y);
        point p = getHough(cx, cy, gx, gy);
        int vx = (int)(p.x * vote.width()  / thetamax),
            vy = (int)(p.y * vote.height() / rhomax);
        vote(vx, vy) += (float)sqrt(gx * gx + gy * gy);
    }
    return vote;
}

// 返回从vote结果得到的直线
std::list<point> getLinesFromVote(CImg<> &vote, point imgsize) {
    double rhomax = sqrt((double)(imgsize.x * imgsize.x + imgsize.y * imgsize.y)) / 2;

    // 对vote高斯平滑后以128为阀值筛选，之后再进行腐蚀操作
    vote.blur((float)sigma);
    cimg_forXY(vote, x, y) if (vote(x, y) < 128) vote(x,y) = 0;
    vote.erode(bound, bound);
    vote.save("v2.bmp");
    // 通过vote中的点得到对应的theta和rho
    std::list<point> ps;
    cimg_forXY(vote, x, y) {
        if (vote(x, y) > 0) {
            double sum = 0, sumx = 0, sumy = 0;
            vote(x, y) = 0;
            std::list< point > l;
            l.push_back(point(x, y));
            while(!l.empty()) {
                int tx = l.front().x, ty = l.front().y;
                l.pop_front();
                sum++; sumx += tx; sumy += ty;
                for (int i = -1; i <= 1; i++) for (int j = -1; j <= 1; j++)
                    if (vote(tx + i, ty + j) > 0) {
                        vote(tx + i, ty + j) = 0;
                        l.push_back(point(tx + i, ty + j));
                    }
            }
            ps.push_back(point( (sumx / sum) * thetamax / vote.width(),
                                (sumy / sum) * rhomax  / vote.height() ));
        }
    }
    return ps;
}

// 由直线得出角点
vector< pair<point, point> >
getPointFromLines(list<point> &lines, vector< pair<point, point> > &re, point imgsize) {
    int imgw = imgsize.x, imgh = imgsize.y;
    // 去除没有平行线的线条
    list<point>::iterator it, it2;
    for (it = lines.begin(); lines.size() > 4 && it != lines.end(); ) {
        bool have = false;
        for (it2 = lines.begin(); it2 != lines.end(); it2++) {
            if (it == it2) continue;
            if (parallel(it->x, it2->x)) {
                have = true;
                break;
            }
        }
        if (!have) it = lines.erase(it);
        else it++;
    }

    // 通过求解直线方程得出交点
    point xy[32];
    int line[32][32], comb[32][2];
    memset(line, 0, sizeof(line));
    int index = 0, ii = 0, ij = 0;
    for (it = lines.begin(), ii = 0; it != lines.end(); it++, ii++) {
        double theta1 = it->x, rho1 = it->y;
        for (it2 = it, ij = 0; it2 != lines.end(); it2++, ij++) {
            double theta2 = it2->x, rho2 = it2->y;
            if (fabs(theta1 - theta2) > cimg::PI / 18) {
                point &pa = xy[index];
                pa.x = round((sin(theta1) * rho2 - sin(theta2) * rho1)
                                / sin(theta1 - theta2));
                if (parallel(theta1, cimg::PI))
                    pa.y = round((rho2 - cos(theta2) * pa.x) / sin(theta2)) + imgh / 2;
                else 
                    pa.y = round((rho1 - cos(theta1) * pa.x) / sin(theta1)) + imgh / 2;
                pa.x += imgw / 2;
                if (pa.x < 0 || pa.x >= imgw || pa.y < 0 || pa.y >= imgh)
                    continue;
                comb[index][0] = ii;
                comb[index][1] = ii + ij;
                index++;
                line[ii][ii + ij] = index;
                line[ii + ij][ii] = index;
            }
        }
    }

    bool valid[32];
    memset(valid, false, sizeof(valid));
    for (int i = 0; i < index; i++)
        if(calcir(i, i, i, index, 4, line, valid)) break;

    int reindex = 0;
    for (int i = 0; i < index; i++) {
        if (!valid[i]) continue;
        int x0 = round(xy[i].x), y0 = round(xy[i].y);
        for (int j = i + 1; j < index; j++) {
            if (!valid[j]) continue;
            if (   comb[i][0] == comb[j][0] || comb[i][0] == comb[j][1]
                || comb[i][1] == comb[j][0] || comb[i][1] == comb[j][1]) {
                int x1 = round(xy[j].x), y1 = round(xy[j].y);
                re.push_back(make_pair(point(x0, y0), point(x1, y1)));
            }
        }
    }
}

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
    angle = angle / 180 * cimg::PI;

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
void drawcicle(CImg<float>& img, point p1, unsigned int r, unsigned int bound) {
    cimg_forXY(img, x, y) {
        point p0(x, y);
        if (fabs(dist(p0, p1) - r) < bound / 2) img(x, y) = 255;
    }
}
