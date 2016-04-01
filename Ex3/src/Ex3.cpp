/**
 * @Author  : ValenW
 * @Link    : https://github.com/ValenW
 * @Email   : ValenW@qq.com
 * @Date    : 2016-03-19 10:34:15
 * @Last Modified by:   ValenW
 * @Last Modified time: 2016-03-31 14:39:03
 */

#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <cstdlib>
#include <list>
#include <vector>

#include "CImg.h"
#include "myImg.h"

using namespace cimg_library;
using namespace std;

unsigned char Blue[3] = {0, 0, 63};
unsigned char mid[1] = {128};

int main() {
    cimg::imagemagick_path("D:\\Program Files\\ImageMagick-6.9.3-Q16\\convert.exe");
    int n;
    string name;
    cin >> n;
    while(n--) {
        calTimeCost();
        cin >> name;
        CImg<> rimg(name.c_str());

        // 定义变量，img的长宽缩小为原图的50%
        // rho, theat为极坐标系下的参数
        CImg<> img = rimg.get_norm().normalize(0, 255).resize(-50, -50, 1, 2, 2);
        unsigned imgw = img.width(), imgh = img.height();
        double rhomax = sqrt((double)(imgh * imgh + imgw * imgw)) / 2,
               thetamax = 2 * cimg::PI;

        // 得到hough投票结果
        CImg<> vote = getVote(img);
        cout << "Vote cost:" << calTimeCost() << endl;
        vote.save((name + "_vote.bmp").c_str());

        // 从投票结果得到直线
        list<point> lines = getLinesFromVote(vote, point(imgw, imgh));
        cout << "getLine cost:" << calTimeCost() << endl;

        // 由直线得到角点
        vector< pair<point, point> > pointPair;
        getPointFromLines(lines, pointPair, point(imgw, imgh));
        cout << "getPoint cost:" << calTimeCost() << endl;
        
        // 在原图上画出求得的矩形
        for (int i = 0; i < pointPair.size(); i++) {
            int x0 = pointPair[i].first.x * 2,
                y0 = pointPair[i].first.y * 2,
                x1 = pointPair[i].second.x * 2,
                y1 = pointPair[i].second.y * 2;
            for (int k = -5; k <= 5; k++)
                rimg.draw_line(x0 + k, y0, x1 + k, y1, Blue);
            for (int k = -5; k <= 5; k++)
                rimg.draw_line(x0, y0 + k, x1, y1 + k, Blue);
            Blue[3] += 64;
        }
        cout << "pointPair cost:" << calTimeCost() << endl;

        rimg.save(name.insert(name.rfind("."), "_draw").c_str());
        cout << "save cost:" << calTimeCost() << endl;

        // 得到原图中四边形的顶点坐标
        point *a4p = new point[4], *srcp = new point[4];
        int l = 0;
        if (dist(pointPair[0]) < dist(pointPair[1])) l = 1;
        srcp[0] = point(pointPair[l].first.x * 2,  pointPair[l].first.y * 2);
        srcp[1] = point(pointPair[l].second.x * 2, pointPair[l].second.y * 2);
        srcp[2] = point(pointPair[3 - l].first.x * 2,  pointPair[3 - l].first.y * 2);
        srcp[3] = point(pointPair[3 - l].second.x * 2, pointPair[3 - l].second.y * 2);

        // 对坐标进行排序，准备映射到A4比例的CImg中
        CImg<> a4;
        if(arrangePoint(srcp)) a4 = CImg<>(2970, 2100, 1, 3, 0);
        else a4 = CImg<>(2100, 2970, 1, 3, 0);
        a4p[0] = point(0, 0);
        a4p[1] = point(a4.width() - 1, 0);
        a4p[2] = point(a4.width() - 1, a4.height() - 1);
        a4p[3] = point(0, a4.height() - 1);
        cout << "cal point cost:" << calTimeCost() << endl;

        // 将原图中的四边形投影映射到A4中
        projectiveMapping(rimg, a4, srcp, a4p);
        cout << "projective mapping cost:" << calTimeCost() << endl;

        a4.save((name + "_a4.jpg").c_str());
        cout << "save cost:" << calTimeCost() << endl;
    }
}
