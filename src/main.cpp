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

#include "aruco.hpp"

#define ESC 27
#define NUM_FRAMES 60

#define CAMERA_WIN "Camera"

using namespace cv;
using namespace std;

void calibrate_camera(String filename, Mat &camMatrix, Mat &distCoeffs);
void detect_arucos(Mat &frame, vector<Aruco> &arucos);
void draw_arucos(Mat &frame, vector<Aruco> &arucos);

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
                
                // Convert the image to gray scale
                // Reduce the size of the frame to reduce processing time
                //
                // Scan of shapes and detect the Arucos
                //
                // Process the data from the Aruco
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
                
                vector<Aruco> arucos;
                output_frame = camera_frame_res;
                
                detect_arucos(camera_frame_bw, arucos);

                RNG rng;
                
                // Convert the image to color to draw onto it
                // cvtColor(output_frame, output_frame, CV_GRAY2BGR);

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

// Calibrate the camera
// See https://docs.opencv.org/2.4/doc/tutorials/calib3d/camera_calibration/camera_calibration.html
// TODO: Use the data from output_calibration to calibrate the camera.
void calibrate_camera(String filename, Mat &camMatrix, Mat &distCoeffs) {
        FileStorage fs(filename, FileStorage::READ);
        
        if(fs.isOpened()) {
                fs["Camera_Matrix"] >> camMatrix;
                fs["Distortion_Coefficients"] >> distCoeffs;
                cout << "Camera calibrated correctly" << endl;
                fs.release();                
        }else {
                cerr << "File \"" + filename + "\" does not exist " << endl;
                exit(1);
        }
}

// Given a camera frame detect the aruco markes
// Give only the border of the aruco
void detect_arucos(Mat &frame, vector<Aruco > &arucos) {
        vector<vector<Point> > contours;
        vector<Vec4i> hierarchy;

        // hierarchy tiene tantos elementos como contornos
        // hierarchy[i][0] es el indice del contorno siguiente al mismo nivel
        // hierarchy[i][1] es el indice del contorno previo al mismo nivel
        // hierarchy[i][2] es el indice del contorno hijo
        // hierarchy[i][3] se el indice del contorno padre
        // Si un indice es -1 es que no existe
        findContours(frame, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);

        for(size_t c = 0; c < contours.size(); ++c) {
                double perimeter = arcLength(contours[c], true);
                double area = contourArea(contours[c]);

                if (area < 500) continue;

                vector<Point> possible_marker;
                approxPolyDP(contours[c], possible_marker, 0.005 * perimeter, true);

                if(possible_marker.size() != 4) continue;
                if(hierarchy[c][2] != -1 && hierarchy[c][3] == -1) continue;
                
                // TODO: Identify aruco dict
                Aruco marker;
                marker.shape = Shape::Cube;
                marker.vertex = possible_marker;

                Moments m = moments(contours[c], true);
                marker.center = Point(m.m10 / m.m00, m.m01 / m.m00);
                
                std::cout << "Marcador: " << possible_marker << endl;
                
                arucos.push_back(marker);
        }
}

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
