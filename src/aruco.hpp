#ifndef _ARUCO_H
#define _ARUCO_H

#include <iostream>
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
        Cube,
        Prism_5,
        Pyramid,
        Pyramid_inv,
        Pyramid_side,
};

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
        
// The Arucos are 6x6 so we have a total of 36 bits
// However, there are only 16 (4x4) bits of useful information
const unsigned char ARUCO_DICTS[NUM_DICTS][4][4] = {
        // Marker 1 [aruco_images/4x4_1000-1.png]
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
