#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <portaudio.h>
#include "fft.hpp"

#define WINDOW_WIDTH 1920
#define WINDOW_HEIGHT 1080
#define SAMPLE_RATE 44100
#define BUFFER_SIZE 512
#define ESCAPE_KEY 27
#define GREEN cv::Scalar(0, 255, 0)
#define WINDOW_NAME "Output"

static CArray fftData(BUFFER_SIZE);

static bool fftDataLock = false;

void drawFFT() {
    cv::Mat output = cv::Mat::zeros(WINDOW_HEIGHT, WINDOW_WIDTH, CV_8UC3);

    // for (int i = 0; i < BUFFER_SIZE; i++) {
    //     std::cout << fftData[i].real() << " ";
    // }
    // std::cout << std::endl;

    fftDataLock = true;

    for (int i = 0; i < BUFFER_SIZE / 2; i++) {
        cv::Point point1(i * 10, WINDOW_HEIGHT);
        cv::Point point2(i * 10 + 5, WINDOW_HEIGHT - abs(fftData[i].real()) * 200);
        cv::rectangle(output, point1, point2, GREEN, cv::FILLED);
    }

    fftDataLock = false;

    imshow(WINDOW_NAME, output);
}

void floatToComplex(const float *array, unsigned long size, CArray *output) {
    for (unsigned long i = 0; i < size; i++) {
        output->operator[](i) = array[i];
    }
}

int paCallback(const void *inputBuffer, void *outputBuffer,
                      unsigned long framesPerBuffer,
                      const PaStreamCallbackTimeInfo *timeInfo,
                      PaStreamCallbackFlags statusFlags,
                      void *userData) {
    auto inputBufferFloat32 = (const float *) inputBuffer;
    auto userDataCArray = (CArray *) userData;

    if (inputBuffer == nullptr) {
        std::cout << "Input buffer is nullptr, listen to the sound of silence" << std::endl;
    }

    if (!fftDataLock) {
        floatToComplex(inputBufferFloat32, framesPerBuffer, userDataCArray);
        fft(fftData);
    }

    return 0;
}

void createAndStartPaInputStream() {
    auto err = Pa_Initialize();

    std::cout << Pa_GetErrorText(err) << std::endl;

    int numDevices = Pa_GetDeviceCount();
    const PaDeviceInfo *deviceInfo;
    for (int i = 0; i < numDevices; i++) {
        deviceInfo = Pa_GetDeviceInfo(i);
        std::cout << i << " " << deviceInfo->name << " " << deviceInfo->defaultSampleRate << " "
                  << deviceInfo->maxInputChannels << " " << deviceInfo->maxOutputChannels << std::endl;
    }

    int device;

    std::cin >> device;

    PaStream *stream;
    PaStreamParameters inputParameters;
    bzero(&inputParameters, sizeof(inputParameters));
    inputParameters.channelCount = 1;
    inputParameters.device = device;
    inputParameters.sampleFormat = paFloat32;
    err = Pa_OpenStream(&stream, &inputParameters, nullptr, SAMPLE_RATE, BUFFER_SIZE, paNoFlag,
                        paCallback, &fftData);

    std::cout << Pa_GetErrorText(err) << std::endl;

    err = Pa_StartStream(stream);

    std::cout << Pa_GetErrorText(err) << std::endl;
}

void terminatePaStream() {
    auto err = Pa_Terminate();
    std::cout << Pa_GetErrorText(err) << std::endl;
}

void test() {
    float floatArray[5] = { 2.1, 3.0, 1.0, 0, 42.5};
    CArray complexArray(5);

    floatToComplex(floatArray, 5, &complexArray);

    for (int i = 0; i < 5; i++) {
        std::cout << complexArray[i] << " ";
    }

    std::cout << std::endl;

    const Complex test[] = {1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 0.0, 0.0};
    CArray data(test, 8);

    fft(data);

    for (int i = 0; i < 8; ++i) {
        std::cout << data[i] << " ";
    }

    std::cout << std::endl;

    const Complex test2[] = {1, 2, 3, 4};
    CArray data2(test2, 4);

    fft(data2);

    for (int i = 0; i < 4; ++i) {
        std::cout << data2[i] << " ";
    }

    std::cout << std::endl;

    cv::namedWindow("Test", 1);

    cv::Mat output = cv::Mat::zeros(120, 350, CV_8UC3);

    cv::rectangle(output, cv::Point(0, 0), cv::Point(20, 20), cv::Scalar(0, 255, 255), cv::FILLED);

    imshow("Test", output);

    cv::waitKey(0);

    cv::destroyWindow("Test");
}

int main() {
    // test();

    cv::namedWindow(WINDOW_NAME, 1);

    createAndStartPaInputStream();

    while(true) {
        drawFFT();

        int key = cv::waitKey(1);

        if (key == ESCAPE_KEY) {
            break;
        }
    }

    terminatePaStream();

    return 0;
}
