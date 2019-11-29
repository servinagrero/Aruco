#ifndef _ARUCO_H
#define _ARUCO_H

#include <map>
#include <vector>

#include <opencv2/core/types.hpp>

using namespace cv;
using namespace std;

// Shape to draw above the Aruco marker
enum class Shape {
        Cube,
        Prism_5,
        Pyramid,
        Pyramid_inv,
        Pyramid_side,
};

// The Arucos are 6x6 so we have a total of 36 bits
// However, there are only 16 (4x4) bits of useful information
const unsigned char ARUCO_DICTS[][4][4] = {
        // Marker 1
        {{255,   0, 255, 255},
         {  0, 255,   0, 255},
         {  0,   0, 255, 255},
         {  0,   0, 255,   0}},

        // Marker 2
        {{  0,   0,   0,   0},
         {255, 255, 255, 255},
         {255,   0,   0, 255},
         {255,   0, 255,   0}}

        // Marker 3

        // Marker 4

        // Marker 5

        // Marker 6

        // Marker 7

        // Marker 8

        // Marker 9

        // Marker 10
};

// Map each Aruco ID to a shape
const map<int, Shape> ARUCO_LUT = {
        {0, Shape::Cube},
        {1, Shape::Pyramid}
};


// Aruco marker
//
// Each marker has:
//   An ID which corresponds to the index in the ARUCO_DICTS
//   4 vertex points
//   A center point
//   A shape to draw above it
struct Aruco {
        char id;
        vector<Point> vertex;
        Point center;
        Shape shape;
};

#endif
