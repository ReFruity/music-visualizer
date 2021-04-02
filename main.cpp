#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/types_c.h>
#include <portaudio.h>

int main() {
    auto err = Pa_Initialize();

    std::cout << err << std::endl;

    cv::namedWindow("Output",1);
    cv::Mat output = cv::Mat::zeros( 120, 350, CV_8UC3 );

    putText(output,
            "Hello World :)",
            cvPoint(15,70),
            cv::FONT_HERSHEY_PLAIN,
            3,
            cvScalar(0,255,0),
            4);

    imshow("Output", output);
    cv::waitKey(0);

    err = Pa_Terminate();
    std::cout << err << std::endl;

    std::cout << "Hello, World!" << std::endl;
    return 0;
}
