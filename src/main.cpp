#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <iterator>
#include <chrono>
#include <cstdint>

#include <opencv2/core/persistence.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/core/utility.hpp>
#include <opencv2/videoio.hpp>

#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/calib3d.hpp>

#include "aruco.hpp"

#define ESC 27
#define NUM_FRAMES 60

#define CAMERA_WIN "Camera"

using namespace cv;
using namespace std;
using namespace std::chrono;

void calibrate_camera(String filename, Mat &camMatrix, Mat &distCoeffs);
void detect_arucos(Mat &frame, vector<Aruco> &arucos);
void draw_arucos(Mat &frame, vector<Aruco> &arucos, Shape current_shape, Mat &camMatrix, Mat &distCoeffs);
char read_marker_dictionary(Mat &aruco_img);
bool compare_matrixes(const uint8_t left_m[4][4], const uint8_t rigth_m[4][4]);

template<class V>
void draw_square(Mat &frame, const vector<V> &v, Scalar color=Scalar(0, 255, 255), int thickness=2);

int main(int argc, char **argv) {

        const String keys =
        "{help h usage ? |          | Print this message      }"
        "{input          |<none>    | Video input file        }"
        "{c              |<none>    | Camera calibration file }"
        "{out            |output.avi| Output video file }";
        
        CommandLineParser cmdParser(argc, argv, keys);

        if (cmdParser.has("help"))
        {
                cmdParser.printMessage();
                return 0;
        }
        // Current shape to draw. Cube by default
        Shape current_shape = Shape::Cube;
        // Camera matrix and distortion coefficients
        Mat camMatrix, distCoeffs;
        
        String fname = cmdParser.get<String>("c");
        calibrate_camera(fname, camMatrix, distCoeffs);

        String input_stream = cmdParser.get<String>("input");
        VideoCapture stream0;

        if(input_stream == "")
                stream0 = cv::VideoCapture(0);
        else
                stream0 = cv::VideoCapture(input_stream);

        String output_file = cmdParser.get<String>("out");

        VideoWriter video_output(output_file, CV_FOURCC('M','J','P','G'), stream0.get(CV_CAP_PROP_FPS),
                Size(stream0.get(CV_CAP_PROP_FRAME_WIDTH), stream0.get(CV_CAP_PROP_FRAME_HEIGHT)));
        cout << "Writing result to video file: " << output_file << endl;
        
        if (!stream0.isOpened()){
                cout << "Cannot open stream" << endl;
                return -1;
        }

        Mat camera_frame, output_frame;
        Mat camera_frame_gray, camera_frame_bw;
        
        // Image used to warp perspective and read the aruco dictionary
        Mat aruco_flat_img(600, 600, CV_8UC3, Scalar(0, 0, 0));
        
        namedWindow(CAMERA_WIN, WINDOW_AUTOSIZE);

        high_resolution_clock::time_point start_t, end_t;
        float fps = 30.0;
        int frame_counter = 0;

        bool running = true;

        vector<Point2f> aruco_flat_vertex;

        aruco_flat_vertex.push_back(Point2f(599,   0));
        aruco_flat_vertex.push_back(Point2f(0,     0));
        aruco_flat_vertex.push_back(Point2f(0,   599));
        aruco_flat_vertex.push_back(Point2f(599, 599));
                                
        while(running) {
                
                if (!stream0.read(camera_frame)) {
                        cout << "Failed to read camera frame" << endl;
                        return -1;
                }
                if(input_stream == "")
                        flip(camera_frame, camera_frame, 1);
                
                // Estimation of the camera fps
                // If working with a video file we can use stream.get(CV_CAP_PROP_FPS)
                if(frame_counter == 0) {
                        start_t = high_resolution_clock::now();
                };
                frame_counter++;
                
                // Convert the camera frame to gray scale
                //
                // Threshold the image to dectect the contours
                //
                // From all contours detected, discard the ones that are not Aruco markers
                // Extract information about the aruco marker
                // 
                // Output the detected Aruco into the frame
                
                cvtColor(camera_frame, camera_frame_gray, CV_BGR2GRAY);

                //
                // Preprocess
                //
                adaptiveThreshold(camera_frame_gray, camera_frame_bw, 255,
                        ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY_INV, 21, 7);

                //
                // Aruco detection
                //
                vector<Aruco> arucos;
                
                detect_arucos(camera_frame_bw, arucos);

                for(auto &aruco: arucos) {                        
                        Mat h = findHomography(aruco.vertex, aruco_flat_vertex);
                        warpPerspective(camera_frame, aruco_flat_img, h, aruco_flat_img.size());
                        aruco.id = read_marker_dictionary(aruco_flat_img);
                }

                //
                // Draw the arucos
                //
                draw_arucos(camera_frame, arucos, current_shape, camMatrix, distCoeffs);
                
                // Calculate the fps to check if the algorithm works in real time
                if(frame_counter == NUM_FRAMES) {
                        end_t = high_resolution_clock::now();
                        duration<double, std::milli> time_span = end_t - start_t;
                        fps = NUM_FRAMES * 1000 / (time_span.count());
                        frame_counter = 0;
                };

                putText(camera_frame, "FPS: " + to_string(fps),
                        cvPoint(15, 40), FONT_HERSHEY_SIMPLEX,
                        0.8, cvScalar(0, 0, 255), 1, CV_AA);

                putText(camera_frame, "Shape: " + to_string(current_shape),
                        cvPoint(10, camera_frame.rows - 10),
                        FONT_HERSHEY_SIMPLEX,
                        0.6, cvScalar(0, 0, 255), 1, CV_AA);
                
                imshow(CAMERA_WIN, camera_frame);
                video_output.write(camera_frame);
                
                // Handle key events
                char key_pressed = waitKey(1);
                switch(key_pressed) {
                        case ESC:
                        case 'q':
                                running = false;
                                break;
                        case 'a':
                                ++current_shape;
                }
                
        }
        stream0.release();
        destroyAllWindows();
}

// Read the text file containing the camera matrix and the distortion coefficients
//
// The first line of the file are the 9 values of the camera matrix.
// The second line are the 5 distortion coefficients
// See https://docs.opencv.org/2.4/doc/tutorials/calib3d/camera_calibration/camera_calibration.html
void calibrate_camera(String filename, Mat &camMatrix, Mat &distCoeffs) {
        ifstream fs(filename);

        if(fs.is_open()) {
                double camData[9], distData[5];
                
                for(int e = 0; e < 9; ++e) fs >> camData[e];
                for(int e = 0; e < 5; ++e) fs >> distData[e];

                Mat camMatrixTemp(3, 3, DataType<double>::type, camData);
                Mat distCoeffsTemp(5, 1, cv::DataType<double>::type, distData);

                camMatrixTemp.copyTo(camMatrix);
                distCoeffsTemp.copyTo(distCoeffs);
                
                cout << camMatrix << endl;
                cout << distCoeffs << endl;
                cout << "Camera calibrated correctly" << endl;

                fs.close();
        } else {
                cerr << "File \"" + filename + "\" does not exist " << endl;
                exit(1);
        }
}

// Given a camera frame detect the aruco markes on it
//
// From all of the contours of the image discard the ones that are no arucos
// For this we assume the following:
//   Arucos are rectangular or square shaped
//   Arucos are of small
//   Arucos have at least one child contour
void detect_arucos(Mat &frame, vector<Aruco > &arucos) {
        
        vector<vector<Point> > contours;
        vector<Vec4i> hierarchy;

        // hierarchy has as many elements as contours there are
        // hierarchy[i][0] is the index of the next contour at the same level
        // hierarchy[i][1] is the index of the previous contour at the same level
        // hierarchy[i][2] is the index of the children contour
        // hierarchy[i][3] is the index of the parent contour
        // If one index is -1 then that element does not exist
        findContours(frame, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);

        for(size_t c = 0; c < contours.size(); ++c) {
                double perimeter = arcLength(contours[c], true);
                double area = contourArea(contours[c]);

                if (area < 500) continue;
                vector<Point> possible_marker;
                approxPolyDP(contours[c], possible_marker, 0.005 * perimeter, true);

                // Discard shapes
                if (possible_marker.size() != 4) continue;
                if (hierarchy[c][2] != -1 && hierarchy[c][3] == -1) continue;
                
                Aruco marker;

                for(auto v : possible_marker) {
                        marker.vertex.push_back(Point2f(v));
                }
                
                Moments m = moments(contours[c], true);
                marker.center = Point2f(double(m.m10 / m.m00), double(m.m01 / m.m00));

                // Push only markers that have been correctly identified
                arucos.push_back(marker);
        }
}

// Given a vector or Aruco markers draw them on the frame
//
// Draw the ID of the marker at its center, the border of the
// marker and its shape above it
void draw_arucos(Mat &frame, vector<Aruco> &arucos, Shape current_shape, Mat &camMatrix, Mat &distCoeffs) {
        
        if(arucos.size() == 0) return;

        for(auto aruco: arucos) {
                if (aruco.id == -1) continue;

                // Rotation and translation matrixes
                Mat rvec, tvec;
        
                // Draw the first border vertex
                aruco.first_vertex = aruco.vertex[aruco.id % 4];
                circle(frame, aruco.first_vertex, 8, Scalar(0, 0, 255), 2);
                
                // Draw id at the center of the marker
                putText(frame, "id=" + to_string(aruco.id),
                        aruco.center, FONT_HERSHEY_SIMPLEX,
                        0.7, cvScalar(0, 0, 255), 1, CV_AA);

                // Draw the border of the marker
                draw_square(frame, aruco.vertex, Scalar(0, 255, 0));

                // Draw the figure of the given marker
                // aruco.shape = ARUCO_LUT.at(aruco.id);
                aruco.shape = current_shape;

                // Projection of the 3d cube points into the camera frame
                vector<Point2d> cube_output_points;
                vector<Point2d> pyramid_output_points;
                vector<Point2d> pyramid_inv_output_points;
                vector<Point2d> pyramid_side_output_points;

                //
                // Cube data
                // 
                // 3d coordinates of the upper face of the cube
                vector<Point3d> cube_3d;
                cube_3d.push_back(Point3d(aruco.vertex[0].x, aruco.vertex[0].y, -250));
                cube_3d.push_back(Point3d(aruco.vertex[1].x, aruco.vertex[1].y, -250));
                cube_3d.push_back(Point3d(aruco.vertex[2].x, aruco.vertex[2].y, -250));
                cube_3d.push_back(Point3d(aruco.vertex[3].x, aruco.vertex[3].y, -250));
                
                // 3d coordinates of the lower face of the cube
                vector<Point3d> cube_points_plain;
                cube_points_plain.push_back(Point3d(aruco.vertex[0].x, aruco.vertex[0].y, 0));
                cube_points_plain.push_back(Point3d(aruco.vertex[1].x, aruco.vertex[1].y, 0));
                cube_points_plain.push_back(Point3d(aruco.vertex[2].x, aruco.vertex[2].y, 0));
                cube_points_plain.push_back(Point3d(aruco.vertex[3].x, aruco.vertex[3].y, 0));

                //
                // Inverted pyramid data
                // 
                // 3d coordinates of the upper face of the cube
                vector<Point3d> pyramid_inv_3d;
                pyramid_inv_3d.push_back(Point3d(aruco.vertex[0].x, aruco.vertex[0].y, -250));
                pyramid_inv_3d.push_back(Point3d(aruco.vertex[1].x, aruco.vertex[1].y, -250));
                pyramid_inv_3d.push_back(Point3d(aruco.vertex[2].x, aruco.vertex[2].y, -250));
                pyramid_inv_3d.push_back(Point3d(aruco.vertex[3].x, aruco.vertex[3].y, -250));
                pyramid_inv_3d.push_back(Point3d((aruco.vertex[0].x + aruco.vertex[2].x)/2, (aruco.vertex[1].y + aruco.vertex[3].y)/2, 0));

                //
                // Pyramid on its side data
                //
                // // 3d coordinates of the upper face of the side pyramid
                vector<Point3d> pyramid_side_3d;
                pyramid_side_3d.push_back(Point3d(aruco.vertex[0].x, aruco.vertex[0].y, -250));
                pyramid_side_3d.push_back(Point3d(aruco.vertex[3].x, aruco.vertex[3].y, -250));
                pyramid_side_3d.push_back(Point3d(aruco.vertex[1].x, (aruco.vertex[1].y + aruco.vertex[2].y)/2, -120));

                //
                // Pyramid data
                //
                // 3d coordinates of the upper face of the pyramid
                // TODO: Calculate the correct point
                vector<Point3d> pyramid_3d;
                pyramid_3d.push_back(Point3d((aruco.vertex[0].x + aruco.vertex[2].x)/2, (aruco.vertex[1].y + aruco.vertex[3].y)/2, -360));

                switch(aruco.shape) {
                        case Shape::Prism_5:
                        case Shape::Pyramid:
                                
                                solvePnP(cube_points_plain, aruco.vertex, camMatrix, distCoeffs, rvec, tvec);
                                projectPoints(pyramid_3d, rvec, tvec, camMatrix, distCoeffs, pyramid_output_points);
                                
                                for(size_t l = 0; l < aruco.vertex.size(); ++l) {
                                        line(frame, aruco.vertex[l], pyramid_output_points[0], Scalar(0, 255, 255), 2);
                                }
                                break;
                                
                        case Shape::Pyramid_side:

                                solvePnP(cube_points_plain, aruco.vertex, camMatrix, distCoeffs, rvec, tvec);
                                projectPoints(pyramid_side_3d, rvec, tvec, camMatrix, distCoeffs, pyramid_side_output_points);

                                line(frame, aruco.vertex[0], pyramid_side_output_points[0], Scalar(0, 255, 255), 2);
                                line(frame, aruco.vertex[3], pyramid_side_output_points[1], Scalar(0, 255, 255), 2);
                                line(frame, pyramid_side_output_points[0], pyramid_side_output_points[1], Scalar(0, 255, 255), 2);

                                line(frame, aruco.vertex[0], pyramid_side_output_points[2], Scalar(0, 255, 255), 2);
                                line(frame, aruco.vertex[3], pyramid_side_output_points[2], Scalar(0, 255, 255), 2);
                                line(frame, pyramid_side_output_points[0], pyramid_side_output_points[2], Scalar(0, 255, 255), 2);
                                line(frame, pyramid_side_output_points[1], pyramid_side_output_points[2], Scalar(0, 255, 255), 2);

                                break;
                                                                
                        case Shape::Pyramid_inv:
                                
                                solvePnP(cube_points_plain, aruco.vertex, camMatrix, distCoeffs, rvec, tvec);
                                projectPoints(pyramid_inv_3d, rvec, tvec, camMatrix, distCoeffs, pyramid_inv_output_points);

                                draw_square(frame, pyramid_inv_output_points);

                                for(size_t l = 0; l < pyramid_inv_output_points.size() - 1; ++l) {
                                        line(frame, pyramid_inv_output_points[l], pyramid_inv_output_points[(l+1)%4], Scalar(0, 255, 255), 2);
                                }
                                
                                for(size_t l = 0; l < pyramid_inv_output_points.size() - 1; ++l) {
                                        line(frame, pyramid_inv_output_points[l], pyramid_inv_output_points[4], Scalar(0, 255, 255), 2);
                                }
                                break;
                                
                        case Shape::Cube:
                                
                                solvePnP(cube_points_plain, aruco.vertex, camMatrix, distCoeffs, rvec, tvec);
                                projectPoints(cube_3d, rvec, tvec, camMatrix, distCoeffs, cube_output_points);

                                // Draw the upper border
                                draw_square(frame, cube_output_points);

                                // Draw vertical lines
                                for(size_t l = 0; l < cube_output_points.size(); ++l) {
                                        line(frame, aruco.vertex[l], cube_output_points[l], Scalar(0, 255, 255), 2);
                                }
                                break;
                                
                        default:
                                break;
                }
        }
}

// Given a flat image containing an aruco extract the data of the marker
//
// Return the id of the aruco if it is found
// Return -1 otherwise
char read_marker_dictionary(Mat &aruco_img) {

        Mat aruco_temp;
        Mat aruco_output;
        
        // Convert to grayscale
        if (aruco_img.channels() > 1)
                cvtColor(aruco_img, aruco_img, CV_BGR2GRAY);

        // Use Otsu to approximate the threshold level
        double thresh = threshold(aruco_img, aruco_temp,
                180, 255, THRESH_BINARY | THRESH_OTSU);
        
        threshold(aruco_temp, aruco_output,
                thresh, 255, THRESH_BINARY);

        // With just 6x6 pixels we have more than enough information
        resize(aruco_output, aruco_output, Size(6, 6));

        uint8_t dict_temp[4][4];
        
        for(int c = 0; c < 4; ++c) {
                for(int r = 0; r < 4; ++r) {
                        dict_temp[c][r] = aruco_output.at<uint8_t>(c+1, r+1);
                }
        }

        for(int m = 0; m < NUM_DICTS; ++m) {
                if(!compare_matrixes(dict_temp, ARUCO_DICTS[m])) continue;
                else return m; // Marker has been found
        }

        return -1; // No id found
}


// Compares the squares matrix of the given size
//
// Returns true if the two matrix are equal
// Returns false otherwise
bool compare_matrixes(const uint8_t left_m[4][4], const uint8_t right_m[4][4]) {
        for(int col = 0; col < 4; ++col) {
                for(int row = 0; row < 4; ++row) {
                        if(left_m[col][row] != right_m[col][row])
                                return false;
                }
        }
        return true;
}

// Given 4 vertex, draw a square with them
template<class V>
void draw_square(Mat &frame, const vector<V> &v, Scalar color, int thickness) {
        if (v.size() != 4) return;

        for(size_t l = 0; l < v.size(); ++l) {
                line(frame, v[l], v[(l + 1) %4], color, thickness);
        }
}
