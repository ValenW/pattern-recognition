/* 
* @Author: ValenW
* @Date:   2016-03-06 09:23:42
* @Last Modified by:   ValenW
* @Last Modified time: 2016-03-09 19:20:00
*/

#include <iostream>
#include <string>
#include <cstdlib>
#include "myImg.h"

int main() {
    unsigned int n = 0;
    std::string outpath;
    char buffer[30];
    std::cin >> n;
    std::cin >> outpath;
    for(int i = 0; i < n; i++) {
        std::string fname;
        std::cin >> fname;
        CImg<unsigned char> a(fname.c_str());

        double angle = 0;
        std::cin >> angle;
        std::string outname = outpath + "\\rotate" + itoa(i + 1, buffer, 10) + ".bmp";
        myrotate(a, angle).save(outname.c_str());
        // a.get_rotate(angle).save(outname.c_str());

        double size = 1;
        std::cin >> size;
        outname = outpath + "\\resize" + itoa(i + 1, buffer, 10) + ".bmp";
        myresize(a, size).save(outname.c_str());
        // a.get_resize(a.width() * size, a.height() * size).save("output\\CImgResize.bmp");
    }

    CImg<unsigned char> b("dataset\\blank.bmp");

    point p1, p2, p3;
    unsigned int bound, r;

    // 0 <= x, y < 1024
    // draw rectangle
    std::cin >> p1.x >> p1.y >> p2.x >> p2.y >> p3.x >> p3.y >> bound;
    drawrectangle(b, p1, p2, p3, bound);

    // draw triangle
    std::cin >> p1.x >> p1.y >> p2.x >> p2.y >> p3.x >> p3.y >> bound;
    drawreiangle(b, p1, p2, p3, bound);

    // draw cicle
    std::cin >> p1.x >> p1.y >> r >> bound;
    drawcicle(b, p1, r, bound);

    b.save((outpath + "\\draw.bmp").c_str());
    return 0;
}
