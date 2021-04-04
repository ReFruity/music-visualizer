#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/types_c.h>
#include <portaudio.h>
#include <valarray>
#include <chrono>
#include <thread>

#define BUFFER_SIZE 256
#define ESCAPE_KEY 27
#define GREEN cv::Scalar(0, 255, 0)

typedef std::complex<double> Complex;
typedef std::valarray<Complex> CArray;

static CArray fftData(BUFFER_SIZE);

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

    floatToComplex(inputBufferFloat32, framesPerBuffer, userDataCArray);

    // for (int i = 0; i < framesPerBuffer; i++) {
    //     std::cout << inputBufferFloat32[i] << " ";
    // }
    // std::cout << std::endl;

    return 0;
}

// https://rosettacode.org/wiki/Fast_Fourier_transform#C.2B.2B
void fft(CArray &x) {
    // DFT
    unsigned int N = x.size(), k = N, n;
    double thetaT = 3.14159265358979323846264338328L / N;
    Complex phiT = Complex(cos(thetaT), -sin(thetaT)), T;
    while (k > 1) {
        n = k;
        k >>= 1;
        phiT = phiT * phiT;
        T = 1.0L;
        for (unsigned int l = 0; l < k; l++) {
            for (unsigned int a = l; a < N; a += n) {
                unsigned int b = a + k;
                Complex t = x[a] - x[b];
                x[a] += x[b];
                x[b] = t * T;
            }
            T *= phiT;
        }
    }
    // Decimate
    unsigned int m = (unsigned int) log2(N);
    for (unsigned int a = 0; a < N; a++) {
        unsigned int b = a;
        // Reverse bits
        b = (((b & 0xaaaaaaaa) >> 1) | ((b & 0x55555555) << 1));
        b = (((b & 0xcccccccc) >> 2) | ((b & 0x33333333) << 2));
        b = (((b & 0xf0f0f0f0) >> 4) | ((b & 0x0f0f0f0f) << 4));
        b = (((b & 0xff00ff00) >> 8) | ((b & 0x00ff00ff) << 8));
        b = ((b >> 16) | (b << 16)) >> (32 - m);
        if (b > a) {
            Complex t = x[a];
            x[a] = x[b];
            x[b] = t;
        }
    }
}

void drawRectangle(cv::Mat &output) {

}

void drawFFT() {
    cv::Mat output = cv::Mat::zeros(1080, 1920, CV_8UC3);

    // putText(output,
    //         std::to_string(fftData[0].real()),
    //         cvPoint(15, 70),
    //         cv::FONT_HERSHEY_PLAIN,
    //         3,
    //         cvScalar(0, 255, 0),
    //         4);

    // for (int i = 0; i < BUFFER_SIZE; i++) {
    //     std::cout << fftData[i].real() << " ";
    // }
    // std::cout << std::endl;

    for (int i = 0; i < BUFFER_SIZE; i++) {
        cv::Point point1(i * 10, 0);
        cv::Point point2(i * 10 + 5, fftData[i].real() * 1000);
        cv::rectangle(output, point1, point2, GREEN, cv::FILLED);
    }

    imshow("Output", output);
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

    // forward fft
    fft(data);

    std::cout << "fft" << std::endl;
    for (int i = 0; i < 8; ++i) {
        std::cout << data[i] << " ";
    }

    std::cout << std::endl;

    cv::namedWindow("Test", 1);

    cv::Mat output = cv::Mat::zeros(120, 350, CV_8UC3);

    cv::rectangle(output, cv::Point(0, 0), cv::Point(20, 20), cv::Scalar(0, 255, 255), cv::FILLED);

    imshow("Test", output);

    cv::waitKey(0);
}

int main() {
    // test();

    auto err = Pa_Initialize();

    std::cout << Pa_GetErrorText(err) << std::endl;

    int numDevices = Pa_GetDeviceCount();
    const PaDeviceInfo *deviceInfo;
    for (int i = 0; i < numDevices; i++) {
        deviceInfo = Pa_GetDeviceInfo(i);
        std::cout << i << " " << deviceInfo->name << " " << deviceInfo->defaultSampleRate << " "
                  << deviceInfo->maxInputChannels << " " << deviceInfo->maxOutputChannels << std::endl;
    }

    PaStream *stream;
    PaStreamParameters inputParameters;
    bzero(&inputParameters, sizeof(inputParameters));
    inputParameters.channelCount = 1;
    inputParameters.device = 1;
    inputParameters.sampleFormat = paFloat32;
    double sampleRate = 44100;
    err = Pa_OpenStream(&stream, &inputParameters, nullptr, sampleRate, BUFFER_SIZE, paNoFlag,
                        paCallback, &fftData);

    std::cout << Pa_GetErrorText(err) << std::endl;

    err = Pa_StartStream(stream);

    std::cout << Pa_GetErrorText(err) << std::endl;

    while(true) {
        drawFFT();

        int key = cv::waitKey(1);

        if (key == ESCAPE_KEY) {
            break;
        }
    }

    err = Pa_Terminate();
    std::cout << Pa_GetErrorText(err) << std::endl;

    return 0;
}
