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

        namedWindow(CAMERA_WIN, WINDOW_AUTOSIZE);

        int slider_scale = 60;
        int slider_thresh = 120;
        createTrackbar("Reduce scale", CAMERA_WIN, &slider_scale, 100);
        createTrackbar("Threshold", CAMERA_WIN, &slider_thresh, 255);

        time_t start_t, end_t;
        int fps = 30;
        int frame_counter = 0;

        bool running = true;
        
        while(running) {
                
                if (!stream0.read(camera_frame)) {
                        std::cout << "Failed to read camera frame" << std::endl;
                        return -1;
                }

                resize(camera_frame, camera_frame_res, Size(),
                        (float)slider_scale / 100, (float)slider_scale / 100);

                cvtColor(camera_frame_res, camera_frame_gray, CV_BGR2GRAY);
                
                GaussianBlur(camera_frame_gray, camera_frame_gray,
                        Size(3, 3), 5);

                threshold(camera_frame_gray, camera_frame_bw,
                        slider_thresh, 255, THRESH_BINARY);

                // Calculation of fps from the camera stream
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

                cvtColor(camera_frame_bw, camera_frame_bw, CV_GRAY2BGR);

                putText(camera_frame_bw, "FPS: " + std::to_string(fps),
                        cvPoint(15, 40), FONT_HERSHEY_SIMPLEX,
                        1, cvScalar(0, 0, 255), 1, CV_AA);
                
                // TODO: Preprocess the camera frame.
                // 
                // Convert the image to gray scale
                // Reduce the size of the frame to facilitate the processing
                //
                // Scan of shapes and detect the Arucos
                //
                // Process the data from the Aruco
                // 
                // Output the detected Aruco into the frame
                
                imshow(CAMERA_WIN, camera_frame_bw);

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
