#include <iostream>
#include <time.h>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>

#define ESC 27
#define NUM_FRAMES 60

#define CAMERA_WIN "Camera"

using namespace cv;

void calibrate_camera();

int main() {

        VideoCapture stream0 = cv::VideoCapture(0);

        if (!stream0.isOpened()){
                std::cout << "Cannot open camera" << std::endl;
                return -1;
        }

        Mat camera_frame, camera_frame_res;
        Mat camera_frame_gray, camera_frame_bw;
        Mat camera_frame_canny, output_frame;

        namedWindow(CAMERA_WIN, WINDOW_AUTOSIZE);

        int slider_scale = 60;
        int slider_thresh = 120;
        int slider_ratio = 20;
        createTrackbar("Reduce scale", CAMERA_WIN, &slider_scale, 100);
        createTrackbar("Threshold", CAMERA_WIN, &slider_thresh, 255);
        createTrackbar("Ratio", CAMERA_WIN, &slider_ratio, 80);

        time_t start_t, end_t;
        int fps = 30;
        int frame_counter = 0;

        bool running = true;
        
        while(running) {
                
                if (!stream0.read(camera_frame)) {
                        std::cout << "Failed to read camera frame" << std::endl;
                        return -1;
                }
                
                // TODO: Preprocess the camera frame.
                // 
                // Convert the image to gray scale
                // Reduce the size of the frame to reduce processing time
                //
                // Scan of shapes and detect the Arucos
                //
                // Process the data from the Aruco
                // 
                // Output the detected Aruco into the frame
                
                resize(camera_frame, camera_frame_res, Size(),
                        (float)slider_scale / 100, (float)slider_scale / 100);

                cvtColor(camera_frame_res, camera_frame_gray, CV_BGR2GRAY);

                GaussianBlur(camera_frame_gray, camera_frame_gray,
                        Size(9, 9), 5);

                // Canny(camera_frame_gray, camera_frame_canny,
                // slider_thresh, slider_thresh*(slider_ratio/10), 3);
                threshold(camera_frame_gray, camera_frame_bw,
                        slider_thresh, 255, THRESH_BINARY);

                Mat img_dilated;
                int erosion_size = 3;
                Mat kernel = getStructuringElement(MORPH_RECT,
                        Size( 2*erosion_size + 1, 2*erosion_size+1 ),
                        Point( erosion_size, erosion_size ) );
        
                dilate(camera_frame_bw, img_dilated, kernel);

                output_frame = img_dilated;

                Mat img_detected;
                Mat labels;
                Mat stats;
                Mat centroids;
                
                connectedComponentsWithStats(output_frame, labels, stats, centroids);

                std::cout << labels.size() << " " << stats.size() << " " << centroids.size() << std::endl;

                RNG rng;
                
                for(int i = 1; i < centroids.rows; ++i) {

                        int width = stats.at<int>(i, CC_STAT_WIDTH);
                        int height = stats.at<int>(i, CC_STAT_HEIGHT);
                        int area = stats.at<int>(i, CC_STAT_AREA);

                        if (area < 1000) continue;
                        std::cout << "Area: " << area << std::endl;
                        
                        // Center point of the rectangle
                        Point c_p(centroids.at<double>(i, 0), centroids.at<double>(i, 1));
                        std::cout << c_p << std::endl;

                        // Upper left point
                        Point v_p1(stats.at<int>(i, CC_STAT_LEFT), stats.at<int>(i, CC_STAT_TOP));

                        // Bottom right point
                        Point v_p2(stats.at<int>(i, CC_STAT_LEFT) + width,
                                stats.at<int>(i, CC_STAT_TOP) + height);
                
                        circle(output_frame, c_p, 3, Scalar(0, 0, 255), -1);
                        rectangle(output_frame, v_p1, v_p2, Scalar(0, 255, 0));
                }

                // Estimation of the camera fps
                // If working with a video file we can use stream.get(CV_CAP_PROP_FPS)
                if(frame_counter == 0) {
                        time(&start_t);
                };
                frame_counter++;
                if(frame_counter == NUM_FRAMES) {
                        time(&end_t);
                        fps = NUM_FRAMES / std::difftime(end_t, start_t);
                        frame_counter = 0;
                };

                cvtColor(output_frame, output_frame, CV_GRAY2BGR);

                putText(output_frame, "FPS: " + std::to_string(fps),
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
void calibrate_camera() {

}
