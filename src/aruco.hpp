#ifndef _ARUCO_H
#define _ARUCO_H

#include <iostream>
#include <sstream>
#include <map>
#include <vector>

#include <opencv2/core/types.hpp>

// Total number of Aruco markers we have
// There are 4 markers per card to acound for rotation
#define NUM_ARUCOS 10
#define NUM_DICTS (NUM_ARUCOS * 4)

using namespace cv;
using namespace std;

// Shape to draw above the Aruco marker
enum class Shape {
        Cube = 0,
        Pyramid,
        Pyramid_inv,
        Pyramid_side,
        Prism_5
};

// Overload ++ operator on shape to cycle between the values
Shape& operator++( Shape &sh ) {
        using IntType = typename std::underlying_type<Shape>::type;
        sh = static_cast<Shape>( static_cast<IntType>(sh) + 1 );
        if ( sh == Shape::Prism_5 )
                sh = static_cast<Shape>(0);
        return sh;
}

// Overload << operator on shape to print as str
ostream& operator<<(ostream& os, const Shape& shape) {
        switch(shape) {
                case Shape::Cube:
                        os << "Cube";
                        break;
                case Shape::Pyramid:
                        os << "Pyramid";
                        break;
                case Shape::Pyramid_inv:
                        os << "Pyramid inv";
                        break;
                case Shape::Pyramid_side:
                        os << "Pyramid side";
                        break;
                case Shape::Prism_5:
                        os << "Prism";
                        break;
        }
        return os;
}

// Overload to_sring function to return a shape as a string
string to_string(Shape sh)
{
    ostringstream os;
    os << sh;
    return os.str();
}

// The Arucos are 6x6 so we have a total of 36 bits
// However, there are only 16 (4x4) bits of useful information
//
// Each marker has 4 dictionaries, one per rotation
// The matrix is encoded clockwise. This way the first vertex of the
// marker is the index of the marker % 4
const unsigned char ARUCO_DICTS[NUM_DICTS][4][4] = {
        // Marker 1 [aruco_images NUM_DICT/4x4_1000-1.png]
        {{0  , 0  , 0  , 0  },
         {255, 255, 255, 255},
         {255, 0  , 0  , 255},
         {255, 0  , 255, 0  }},

        {{0  , 255, 255, 0  },
         {0  , 255, 0  , 255},
         {0  , 255, 0  , 0  },
         {0  , 255, 255, 255}},

        {{0  , 255, 0  , 255},
         {255, 0  , 0  , 255},
         {255, 255, 255, 255},
         {0  , 0  , 0  , 0  }},

        {{255, 255, 255, 0  },
         {0  , 0  , 255, 0  },
         {255, 0  , 255, 0  },
         {0  , 255, 255, 0  }},

// Marker 2 [aruco_images/4x4_1000-0.png]
        {{255, 0  , 255, 255},
         {0  , 255, 0  , 255},
         {0  , 0  , 255, 255},
         {0  , 0  , 255, 0  }},

        {{255, 255, 255, 0  },
         {255, 0  , 255, 255},
         {0  , 255, 0  , 0  },
         {255, 0  , 0  , 0  }},

        {{0  , 255, 0  , 0  },
         {255, 255, 0  , 0  },
         {255, 0  , 255, 0  },
         {255, 255, 0  , 255}},

        {{0  , 0  , 0  , 255},
         {0  , 0  , 255, 0  },
         {255, 255, 0  , 255},
         {0  , 255, 255, 255}},

// Marker 3 [aruco_images/4x4_1000-8.png]
        {{255, 255, 255, 255},
         {255, 255, 255, 0  },
         {255, 255, 0  , 255},
         {255, 0  , 255, 0  }},

        {{255, 0  , 255, 0  },
         {255, 255, 0  , 255},
         {255, 255, 255, 0  },
         {255, 255, 255, 255}},

        {{0  , 255, 0  , 255},
         {255, 0  , 255, 255},
         {0  , 255, 255, 255},
         {255, 255, 255, 255}},

        {{255, 255, 255, 255},
         {0  , 255, 255, 255},
         {255, 0  , 255, 255},
         {0  , 255, 0  , 255}},

// Marker 4 [aruco_images/4x4_1000-2.png]
        {{0  , 0  , 255, 255},
         {0  , 0  , 255, 255},
         {0  , 0  , 255, 0  },
         {255, 255, 0  , 255}},

        {{255, 255, 0  , 255},
         {255, 255, 255, 0  },
         {0  , 0  , 0  , 255},
         {0  , 0  , 0  , 255}},

        {{255, 0  , 255, 255},
         {0  , 255, 0  , 0  },
         {255, 255, 0  , 0  },
         {255, 255, 0  , 0  }},

        {{255, 0  , 0  , 0  },
         {255, 0  , 0  , 0  },
         {0  , 255, 255, 255},
         {255, 0  , 255, 255}},

// Marker 5 [aruco_images/4x4_1000-3.png]
        {{255, 0  , 0  , 255},
         {255, 0  , 0  , 255},
         {0  , 255, 0  , 0  },
         {0  , 255, 255, 0  }},

        {{255, 255, 0  , 0  },
         {0  , 0  , 0  , 255},
         {0  , 0  , 255, 255},
         {255, 255, 0  , 0  }},

        {{0  , 255, 255, 0  },
         {0  , 0  , 255, 0  },
         {255, 0  , 0  , 255},
         {255, 0  , 0  , 255}},

        {{0  , 0  , 255, 255},
         {255, 255, 0  , 0  },
         {255, 0  , 0  , 0  },
         {0  , 0  , 255, 255}},

// Marker 6 [aruco_images/4x4_1000-9.png]
        {{255, 255, 0  , 0  },
         {255, 255, 255, 255},
         {0  , 255, 0  , 255},
         {0  , 255, 255, 0  }},

        {{0  , 255, 255, 0  },
         {0  , 255, 0  , 255},
         {255, 255, 255, 255},
         {255, 255, 0  , 0  }},

        {{0  , 255, 255, 0  },
         {255, 0  , 255, 0  },
         {255, 255, 255, 255},
         {0  , 0  , 255, 255}},

        {{0  , 0  , 255, 255},
         {255, 255, 255, 255},
         {255, 0  , 255, 0  },
         {0  , 255, 255, 0  }},

// Marker 7 [aruco_images/4x4_1000-4.png]
        {{0  , 255, 0  , 255},
         {0  , 255, 0  , 0  },
         {255, 0  , 0  , 255},
         {255, 255, 255, 0  }},

        {{255, 0  , 255, 0  },
         {0  , 0  , 0  , 255},
         {255, 255, 0  , 255},
         {0  , 0  , 255, 255}},

        {{0  , 255, 255, 255},
         {255, 0  , 0  , 255},
         {0  , 0  , 255, 0  },
         {255, 0  , 255, 0  }},

        {{255, 255, 0  , 0  },
         {255, 0  , 255, 255},
         {255, 0  , 0  , 0  },
         {0  , 255, 0  , 255}},

// Marker 8 [aruco_images/4x4_1000-6.png]
        {{255, 0  , 0  , 255},
         {255, 255, 255, 0  },
         {0  , 0  , 255, 0  },
         {255, 255, 255, 0  }},

        {{255, 0  , 0  , 0  },
         {0  , 255, 255, 255},
         {0  , 255, 0  , 255},
         {255, 255, 0  , 255}},

        {{0  , 255, 255, 255},
         {0  , 255, 0  , 0  },
         {0  , 255, 255, 255},
         {255, 0  , 0  , 255}},

        {{255, 0  , 255, 255},
         {255, 0  , 255, 0  },
         {255, 255, 255, 0  },
         {0  , 0  , 0  , 255}},

// Marker 9 [aruco_images/4x4_1000-5.png]
        {{0  , 255, 255, 255},
         {255, 0  , 0  , 255},
         {255, 255, 0  , 0  },
         {255, 255, 0  , 255}},

        {{255, 255, 0  , 255},
         {255, 0  , 0  , 0  },
         {255, 0  , 255, 255},
         {0  , 255, 255, 255}},

        {{255, 0  , 255, 255},
         {0  , 0  , 255, 255},
         {255, 0  , 0  , 255},
         {255, 255, 255, 0  }},

        {{255, 255, 255, 0  },
         {255, 255, 0  , 255},
         {0  , 0  , 0  , 255},
         {255, 0  , 255, 255}},

// Marker 10 [aruco_images/4x4_1000-7.png]
        {{255, 255, 0  , 0  },
         {0  , 255, 0  , 0  },
         {255, 255, 255, 255},
         {0  , 0  , 255, 0  }},

        {{0  , 0  , 255, 0  },
         {0  , 0  , 255, 255},
         {255, 255, 255, 0  },
         {255, 0  , 255, 0  }},

        {{0  , 255, 0  , 0  },
         {255, 255, 255, 255},
         {0  , 0  , 255, 0  },
         {0  , 0  , 255, 255}},

        {{0  , 255, 0  , 255},
         {0  , 255, 255, 255},
         {255, 255, 0  , 0  },
         {0  , 255, 0  , 0  }},
};

// Map each Aruco ID to a shape
// Currently there are only 4 shapes to chose from
const map<int, Shape> ARUCO_LUT = {
        { 0, Shape::Cube},
        { 1, Shape::Cube},
        { 2, Shape::Cube},
        { 3, Shape::Cube},

        { 4, Shape::Pyramid},
        { 5, Shape::Pyramid},
        { 6, Shape::Pyramid},
        { 7, Shape::Pyramid},

        { 8, Shape::Pyramid_inv},
        { 9, Shape::Pyramid_inv},
        {10, Shape::Pyramid_inv},
        {11, Shape::Pyramid_inv},

        {12, Shape::Pyramid_side},
        {13, Shape::Pyramid_side},
        {14, Shape::Pyramid_side},
        {15, Shape::Pyramid_side},

        {16, Shape::Pyramid_side},
        {17, Shape::Pyramid_side},
        {18, Shape::Pyramid_side},
        {19, Shape::Pyramid_side},

        {20, Shape::Pyramid_side},
        {21, Shape::Pyramid_side},
        {22, Shape::Pyramid_side},
        {23, Shape::Pyramid_side},

        {20, Shape::Pyramid_side},
        {21, Shape::Pyramid_side},
        {22, Shape::Pyramid_side},
        {23, Shape::Pyramid_side},

        {24, Shape::Pyramid_side},
        {25, Shape::Pyramid_side},
        {26, Shape::Pyramid_side},
        {27, Shape::Pyramid_side},

        {28, Shape::Pyramid_side},
        {29, Shape::Pyramid_side},
        {30, Shape::Pyramid_side},
        {31, Shape::Pyramid_side},

        {32, Shape::Pyramid_side},
        {33, Shape::Pyramid_side},
        {34, Shape::Pyramid_side},
        {35, Shape::Pyramid_side},

        {36, Shape::Pyramid_side},
        {37, Shape::Pyramid_side},
        {38, Shape::Pyramid_side},
        {39, Shape::Pyramid_side},
};


// Aruco marker
//
// Each marker has:
//   An id which corresponds to the index in the ARUCO_DICTS
//   4 vertex points
//   A center point
//   A shape to draw above it. This shape is extracted from the ARUCO_LUT using the id.
struct Aruco {
        char id;
        vector<Point2f> vertex;
        Point2f first_vertex;
        Point center;
        Shape shape;
};

#endif
