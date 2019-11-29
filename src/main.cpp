#include <iostream>
#include <time.h>
#include <string>
#include <vector>

#include <opencv2/core/persistence.hpp>
#include <opencv2/core/types.hpp>

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

void calibrate_camera(String filename, Mat &camMatrix, Mat &distCoeffs);
void detect_arucos(Mat &frame, vector<Aruco> &arucos);
void draw_arucos(Mat &frame, vector<Aruco> &arucos);
void read_marker_dictionary(Mat &aruco_img, Mat &aruco_output);


int main(int argc, char **argv) {

        Mat camMatrix, distCoeffs;
        vector<Point> rvecs, tvecs;
        String fname = (argc == 2) ? argv[1] : "output_calibration.xml";
        
        calibrate_camera(fname, camMatrix, distCoeffs);
        
        VideoCapture stream0 = cv::VideoCapture(0);

        if (!stream0.isOpened()){
                cout << "Cannot open camera" << endl;
                return -1;
        }

        Mat camera_frame, camera_frame_res;
        Mat camera_frame_gray, camera_frame_bw;
        Mat camera_frame_canny, output_frame, camera_frame_blur;

        // Image used to warp perspective and read the aruco dictionary
        Mat aruco_img(600, 600, CV_8UC3, Scalar(0, 0, 0));
        Mat aruco_output(600, 600, CV_8UC3, Scalar(0, 0, 0));

        // Points used to create a flat image of the aruco
        vector<Point2f> pts_dst;
        pts_dst.push_back(Point2f(0,     0));
        pts_dst.push_back(Point2f(599,   0));
        pts_dst.push_back(Point2f(599, 599));
        pts_dst.push_back(Point2f(0,   599));
        
        namedWindow(CAMERA_WIN, WINDOW_AUTOSIZE);

        int slider_scale = 100;
        createTrackbar("Reduce scale", CAMERA_WIN, &slider_scale, 100);

        time_t start_t, end_t;
        int fps = 30;
        int frame_counter = 0;

        bool running = true;
        
        while(running) {
                
                if (!stream0.read(camera_frame)) {
                        cout << "Failed to read camera frame" << endl;
                        return -1;
                }

                flip(camera_frame, camera_frame, 1);
                
                // Estimation of the camera fps
                // If working with a video file we can use stream.get(CV_CAP_PROP_FPS)
                if(frame_counter == 0) {
                        time(&start_t);
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

                // We reduce the size of the frame to ease processing
                resize(camera_frame, camera_frame_res, Size(),
                        (float)slider_scale / 100, (float)slider_scale / 100);

                cvtColor(camera_frame_res, camera_frame_gray, CV_BGR2GRAY);
                
                //
                // Preprocess
                //
                adaptiveThreshold(camera_frame_gray, camera_frame_bw, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY_INV, 7, 7);

                //
                // Aruco detection
                //
                vector<Aruco> arucos;
                output_frame = camera_frame_res;
                
                detect_arucos(camera_frame_bw, arucos);

                if(arucos.size() > 0) {        
                        Mat h = findHomography(arucos[0].vertex, pts_dst);
                        warpPerspective(camera_frame_res, aruco_img, h, aruco_img.size());
                        read_marker_dictionary(aruco_img, aruco_output);
                }

                imshow("Aruco image", aruco_output);

                //
                // Draw the arucos
                // 
                draw_arucos(output_frame, arucos);
                
                // Calculate the fps to check if the algorithm works in real time
                if(frame_counter == NUM_FRAMES) {
                        time(&end_t);
                        fps = NUM_FRAMES / difftime(end_t, start_t);
                        frame_counter = 0;
                };

                putText(output_frame, "FPS: " + to_string(fps),
                        cvPoint(15, 40), FONT_HERSHEY_SIMPLEX,
                        1, cvScalar(0, 0, 255), 1, CV_AA);

                imshow(CAMERA_WIN, output_frame);

                // Handle key events
                char key_pressed = waitKey(1);
                switch(key_pressed) {
                        case ESC:
                        case 'q':
                                running = false;
                                break;
                }
                
        }
        stream0.release();
        destroyAllWindows();
}

// Read the xml file containing the camera matrix and the distortion coefficients
//
// See https://docs.opencv.org/2.4/doc/tutorials/calib3d/camera_calibration/camera_calibration.html
// TODO: Use the data from output_calibration to calibrate the camera.
void calibrate_camera(String filename, Mat &camMatrix, Mat &distCoeffs) {
        FileStorage fs(filename, FileStorage::READ);
        
        if(fs.isOpened()) {
                fs["Camera_Matrix"] >> camMatrix;
                fs["Distortion_Coefficients"] >> distCoeffs;
                cout << "Camera calibrated correctly" << endl;
                fs.release();                
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
                
                if(possible_marker.size() != 4) continue;
                if (hierarchy[c][2] != -1 && hierarchy[c][3] == -1) continue;
               
                // TODO: Identify aruco dict
                Aruco marker;
                marker.shape = Shape::Cube;
                marker.vertex = possible_marker;

                Moments m = moments(contours[c], true);
                marker.center = Point(m.m10 / m.m00, m.m01 / m.m00);
                
                arucos.push_back(marker);
        }
}

// Given a vector or Aruco markers draw them on the frame
//
// Draw the ID of the marker at its center, the border of the
// marker and its shape above it
void draw_arucos(Mat &frame, vector<Aruco> &arucos) {
        
        if(arucos.size() == 0) return;

        for(auto aruco: arucos) {
                // Draw center  and vertex point
                circle(frame, aruco.center, 3, Scalar(0, 0, 255), -1);

                for(size_t side = 0; side < aruco.vertex.size(); ++side) {
                        line(frame, aruco.vertex[side], aruco.vertex[(side + 1) % 4],
                                Scalar(0, 255, 0), 2);
                }

                // Draw figure
        }
}

// Given a flat image containing an aruco extract the data of the marker
//
// We only need to read 16 (4x4) bits of information.
// However this is sensitive to rotation
void read_marker_dictionary(Mat &aruco_img, Mat &aruco_output) {
        
        // FIXME: Improve segmetation of the marker
        threshold(aruco_img, aruco_output, 200, 255, THRESH_BINARY);

        Point point;
        // TODO: Read the 16 values
        for(int c = 1; c < 7; ++c) {
                for(int r = 1; r < 7; ++r) {
                        point.x = 100 * (r-1) + 50;
                        point.y = 100 * (c-1) + 50;
                        circle(aruco_output, point, 3, Scalar(0, 0, 255), -1);
                }
        }

        // Draw lines to mark each bit
        // For debug purposes only
        for(int c = 1; c < 6; ++c) {
                // Vertical
                line(aruco_output, Point(100*c, 0), Point(100*c, 599), Scalar(0, 255, 0), 2);
                // Horizontal
                line(aruco_output, Point(0, 100*c), Point(599, 100*c), Scalar(0, 255, 0), 2);                        
        }
}
