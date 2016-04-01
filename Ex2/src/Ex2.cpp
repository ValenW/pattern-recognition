/**
 * @Author  : ValenW
 * @Link    : https://github.com/ValenW
 * @Email   : ValenW@qq.com
 * @Date    : 2016-03-19 10:34:15
 * @Last Modified by:   ValenW
 * @Last Modified time: 2016-03-27 22:17:42
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

const unsigned char Blue[3] = {0, 0, 255};
const unsigned char mid[1] = {128};

int main() {
    cimg::imagemagick_path("D:\\Program Files\\ImageMagick-6.9.3-Q16\\convert.exe");
    int n;
    string name;
    cin >> n;
    while(n--) {
        calTimeCost();
        cin >> name;
        CImg<unsigned char> rimg(name.c_str());

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
        vector< pair<point, point> > draw;
        getPointFromLines(lines, draw, point(imgw, imgh));
        cout << "getPoint cost:" << calTimeCost() << endl;
        
        // 在原图上画出求得的矩形
        for (int i = 0; i < draw.size(); i++) {
            int x0 = draw[i].first.x * 2,
                y0 = draw[i].first.y * 2,
                x1 = draw[i].second.x * 2,
                y1 = draw[i].second.y * 2;
            for (int k = -5; k <= 5; k++)
                rimg.draw_line(x0 + k, y0, x1 + k, y1, Blue);
            for (int k = -5; k <= 5; k++)
                rimg.draw_line(x0, y0 + k, x1, y1 + k, Blue);
        }
        cout << "draw cost:" << calTimeCost() << endl;

        rimg.save(name.insert(name.rfind("."), "_draw").c_str());
        cout << "save cost:" << calTimeCost() << endl;
    }
}
