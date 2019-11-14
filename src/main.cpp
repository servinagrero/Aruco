#include <iostream>
#include <time.h>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>

#define ESC 27
#define NUM_FRAMES 30

using namespace cv;

int main(int argc, char **argv) {

        VideoCapture stream1 = cv::VideoCapture(0);

        if (!stream1.isOpened()){
                std::cout << "Cannot open camera" << std::endl;
                return -1;
        }

        Mat camera_frame;

        time_t start, end;
        int fps = 30;
        int counter = 0;
        
        while(true) {
                stream1.read(camera_frame);
                
                if(counter == 0) {
                        time(&start);
                };
                counter++;
                if(counter == NUM_FRAMES) {
                        time(&end);
                        fps = NUM_FRAMES / std::difftime(end, start);
                        counter = 0;
                };

                putText(camera_frame, "FPS: " + std::to_string(fps), cvPoint(15, 40), 
                        FONT_HERSHEY_SIMPLEX,
                        1, cvScalar(0, 0,255), 1, CV_AA);
                
                imshow("Camera", camera_frame);

                char key = waitKey(1);
                if (key == 'q') {
                        break;
                }
                
        }
        destroyWindow("Camera");
}
