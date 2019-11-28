#include <iostream>
#include <time.h>
#include <string>

#include <opencv2/core/persistence.hpp>

#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#define ESC 27
#define NUM_FRAMES 60

#define CAMERA_WIN "Camera"

using namespace cv;

void calibrate_camera(String filename, Mat &camMatrix, Mat &distCoeffs);

int main(int argc, char **argv) {

        Mat camMatrix, distCoeffs;
        String fname = (argc == 2) ? argv[1] : "output_calibration.xml";
        
        calibrate_camera(fname, camMatrix, distCoeffs);

        VideoCapture stream0 = cv::VideoCapture(0);

        if (!stream0.isOpened()){
                std::cout << "Cannot open camera" << std::endl;
                return -1;
        }

        Mat camera_frame, camera_frame_res;
        Mat camera_frame_gray, camera_frame_bw;
        Mat camera_frame_canny, output_frame, camera_frame_blur;

        namedWindow(CAMERA_WIN, WINDOW_AUTOSIZE);

        int slider_scale = 100;
        int slider_thresh = 120;
        int slider_ratio = 20;
        int slider_hough = 80;
        createTrackbar("Reduce scale", CAMERA_WIN, &slider_scale, 100);
        createTrackbar("Threshold", CAMERA_WIN, &slider_thresh, 255);
        createTrackbar("Ratio", CAMERA_WIN, &slider_ratio, 80);
        createTrackbar("Hough", CAMERA_WIN, &slider_hough, 200);

        time_t start_t, end_t;
        int fps = 30;
        int frame_counter = 0;

        bool running = true;
        
        while(running) {
                
                if (!stream0.read(camera_frame)) {
                        std::cout << "Failed to read camera frame" << std::endl;
                        return -1;
                }

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

                // GaussianBlur(camera_frame_gray, camera_frame_blur, Size(5, 5), -100);

                // threshold(camera_frame_gray, camera_frame_bw, slider_thresh, 255, THRESH_BINARY);
                adaptiveThreshold(camera_frame_gray, camera_frame_bw, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY_INV, 7, 7);
                
                output_frame = camera_frame_res;

                std::vector<std::vector<Point> > contours;
                std::vector<Vec4i> hierarchy;
                findContours(camera_frame_bw, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);

                // Mat labels;
                // Mat stats;
                // Mat centroids;                
                // connectedComponentsWithStats(output_frame, labels, stats, centroids);

                // std::cout << labels.size() << " " << stats.size() << " " << centroids.size() << std::endl;
                RNG rng;

                // Convert the image to color to draw onto it
                // cvtColor(output_frame, output_frame, CV_GRAY2BGR);

                for( size_t i = 0; i < contours.size(); i++ )
                {
                        double arc_len = arcLength(contours[i], true);
                        double area = contourArea(contours[i]);
                        if (area < 500) continue;
                        
                        std::vector<Point> aruco;
                        approxPolyDP(contours[i], aruco, 0.005 * arc_len, true);

                        if (aruco.size() != 4) continue;
                        if (hierarchy[i][2] == -1 && hierarchy[i][3] == -1) continue;
                        
                        // Scalar color = Scalar( rng.uniform(0, 256), rng.uniform(0,256), rng.uniform(0,256) );
                        // std::cout << aruco << std::endl;
                        drawContours(output_frame, contours, (int)i, Scalar(0, 255, 0), 2, LINE_8, hierarchy, 0 );

                        circle(output_frame, aruco[0], 4, Scalar(0, 0, 255), -1);
                        circle(output_frame, aruco[1], 4, Scalar(0, 0, 255), -1);
                        circle(output_frame, aruco[2], 4, Scalar(0, 0, 255), -1);
                        circle(output_frame, aruco[3], 4, Scalar(0, 0, 255), -1);

                }
                
                // for(int i = 1; i < centroids.rows; ++i) {
                        
                //         int area = stats.at<int>(i, CC_STAT_AREA);

                //         // TODO: Mejorar el threshold de figuras detectadas
                //         if (area < 5000 || area > 40000) continue;

                //         int width = stats.at<int>(i, CC_STAT_WIDTH);
                //         int height = stats.at<int>(i, CC_STAT_HEIGHT);
                        
                //         std::cout << "Area: " << area << std::endl;
                        
                //         // Center point of the rectangle
                //         Point center_p(centroids.at<double>(i, 0), centroids.at<double>(i, 1));

                //         // Upper left point
                //         Point upper_p(stats.at<int>(i, CC_STAT_LEFT), stats.at<int>(i, CC_STAT_TOP));

                //         // Bottom right point
                //         Point bottom_p(stats.at<int>(i, CC_STAT_LEFT) + width,
                //                 stats.at<int>(i, CC_STAT_TOP) + height);
                        
                //         drawMarker(output_frame, center_p, Scalar(0, 255, 0));
                //         // circle(camera_frame_res, c_p, 3, Scalar(0, 0, 255), -1);
                //         rectangle(output_frame, upper_p, bottom_p, Scalar(0, 255, 0));
                // }

                // Calculate the fps to check if the algorithm works in real time
                if(frame_counter == NUM_FRAMES) {
                        time(&end_t);
                        fps = NUM_FRAMES / std::difftime(end_t, start_t);
                        frame_counter = 0;
                };

                putText(camera_frame_res, "FPS: " + std::to_string(fps),
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
                std::cout << "Camera calibrated correctly" << std::endl;
                fs.release();                
        }else {
                std::cerr << "File \"" + filename + "\" does not exist " << std::endl;
                exit(1);
        }
}
