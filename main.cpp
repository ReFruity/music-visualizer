#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <portaudio.h>
#include "fft.hpp"

#define LOG_BASE 10
#define WINDOW_WIDTH 1920
#define WINDOW_HEIGHT 1080
#define SAMPLE_RATE 44100
#define BUFFER_SIZE 256
#define ESCAPE_KEY 27
#define GREEN cv::Scalar(0, 255, 0)
#define WINDOW_NAME "Output"

static CArray fftData(BUFFER_SIZE);

static bool fftDataLock = false;

double logBase(double x) {
    return log(x) / log(LOG_BASE);
}

double transformIndex(int i, int from, int to) {
    auto logI = logBase(i);
    auto result = logI;
    return result / logBase(to) * to;
}

Complex interpolate(const CArray &array, double i) {
    return array[floor(i)];
}

void copyCArray(const CArray &source, CArray &destination) {
    if (source.size() != destination.size()) {
        throw std::runtime_error("Source size is not equal to destination size. Cannot copy.");
    }

    for (int i = 0; i < source.size(); i++) {
        destination[i] = source[i];
    }
}

void convertToLogScale(CArray &array) {
    CArray result(array.size());

    for (int i = 0; i < array.size(); i++) {
        auto transformedIndex = transformIndex(i, 0, array.size());
        auto interpolated = interpolate(array, transformedIndex);
        result[i] = interpolated;
    }

    copyCArray(result, array);
}

void absScale(CArray &array) {
    for (int i = 0; i < array.size(); i++) {
        array[i] = Complex(abs(array[i].real()) * 100, array[i].imag());
    }
}

float degreesToRadians(float x) {
    return x * PI / 180.0;
}

float radiansToDegrees(float x) {
    return x * 180.0 / PI;
}

cv::Point offset(const cv::Point &point, float r, float phi) {
    auto radians = degreesToRadians(phi);
    return cv::Point(point.x + r * cos(radians), point.y + r * sin(radians));
}

void drawRotatedRectangle(cv::Mat& image, const cv::Point &base, const cv::Size &rectangleSize, float rotationDegrees) {
    auto AO = rectangleSize.width / 2.0;
    auto AD = rectangleSize.height / 2.0;
    auto pointA = offset(base, AO, 90 + rotationDegrees);
    auto pointB = offset(base, AO, rotationDegrees - 90);
    auto OD = sqrt(AO * AO + AD * AD);
    auto theta = radiansToDegrees(asin(AO / OD));
    auto pointD = offset(base, OD, theta + rotationDegrees);
    auto pointC = offset(base, OD, rotationDegrees - theta);

    cv::Point vertices[4] = { pointA, pointB, pointC, pointD };

    cv::fillConvexPoly(image, vertices, 4, GREEN);
}

void drawFFT() {
    cv::Mat output = cv::Mat::zeros(WINDOW_HEIGHT, WINDOW_WIDTH, CV_8UC3);

    // for (int i = 0; i < BUFFER_SIZE; i++) {
    //     std::cout << fftData[i].real() << " ";
    // }
    // std::cout << std::endl;

    fftDataLock = true;

    auto center = cv::Point(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);
    auto centerCircleRadius = 100;

    // auto height = 1000;
    // auto angle = 0;
    // auto rectangleCenter = offset(center, centerCircleRadius, angle);
    // auto size = cv::Size(20, height);
    // drawRotatedRectangle(output, rectangleCenter, size, angle);

    for (int i = 10; i < BUFFER_SIZE / 2; i++) {
        auto height = fftData[i].real();
        auto angle = (i - 10) / 118.0 * 360.0;
        auto rectangleCenter = offset(center, centerCircleRadius, angle);
        auto size = cv::Size(1, height);
        drawRotatedRectangle(output, rectangleCenter, size, angle);
    }

    for (int i = 0; i < BUFFER_SIZE / 2; i++) {
        cv::Point point1(i * 10, WINDOW_HEIGHT);
        cv::Point point2(i * 10 + 5, WINDOW_HEIGHT - fftData[i].real());
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
        convertToLogScale(fftData);
        absScale(fftData);
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

    int device = 4;

    // std::cin >> device;

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
    auto transformedIndex = transformIndex(1, 1, 100);

    std::cout << transformedIndex << std::endl;

    transformedIndex = transformIndex(10, 1, 100);

    std::cout << transformedIndex << std::endl;

    transformedIndex = transformIndex(50, 1, 100);

    std::cout << transformedIndex << std::endl;

    transformedIndex = transformIndex(90, 1, 100);

    std::cout << transformedIndex << std::endl;

    CArray cArray1({ 0, 1, 2, 3, 4, 5 });
    convertToLogScale(cArray1);

    for (int i = 0; i < 6; i++) {
        std::cout << cArray1[i] << " ";
    }

    std::cout << std::endl;

    CArray cArray({ 1, 10, 100, 1000, 10000, 100000 });
    convertToLogScale(cArray);

    for (int i = 0; i < 6; i++) {
        std::cout << cArray[i] << " ";
    }

    std::cout << std::endl;

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
