#ifndef _ARUCO_H
#define _ARUCO_H

#include <map>
#include <vector>

#include <opencv2/core/types.hpp>

using namespace cv;
using namespace std;

enum class Shape {
        Cube,
        Prism_5,
        Pyramid,
        Pyramid_inv,
        Pyramid_side,
};

// The Arucos are 6x6 so we have a total of 36 bits

const char ARUCO_DICTS[][6][6] = {
        {{0, 0, 0, 0, 0, 0},
         {0, 1, 0, 1, 1, 0},
         {0, 0, 1, 0, 1, 0},
         {0, 0, 0, 1, 1, 0},
         {0, 0, 0, 1, 0, 0},
         {0, 0, 0, 0, 0, 0}},

        
};

struct Aruco {
        vector<Point> vertex;
        Point center;
        Shape shape;
        char dict[6][6];
};

#endif
