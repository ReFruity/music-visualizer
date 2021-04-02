#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/types_c.h>
#include <portaudio.h>

typedef struct
{
    float left_phase;
    float right_phase;
}
paData;

static paData *data1;

static int paCallback(const void *inputBuffer, void *outputBuffer,
                      unsigned long framesPerBuffer,
                      const PaStreamCallbackTimeInfo* timeInfo,
                      PaStreamCallbackFlags statusFlags,
                      void *userData )
{
    auto inputBufferFloat32 = (const float *) inputBuffer;

    // std::cout << "-----" << std::endl;
    // std::cout << framesPerBuffer << std::endl;
    // std::cout << sizeof(*inputBufferFloat32) << std::endl;
    // std::cout << sizeof(inputBufferFloat32[0]) << std::endl;
    // std::cout << inputBufferFloat32[0] << std::endl;

    if (inputBuffer == nullptr) std::cout << "Input buffer is nullptr, listen to the sound of silence" << std::endl;

    for (int i = 0; i < framesPerBuffer; i++) {
        std::cout << *inputBufferFloat32++ << " ";
    }

    std::cout << std::endl;

    return 0;
}

int main() {
    auto err = Pa_Initialize();

    std::cout << Pa_GetErrorText(err) << std::endl;

    int numDevices = Pa_GetDeviceCount();
    if (numDevices < 0) {
        printf( "ERROR: Pa_CountDevices returned 0x%x\n", numDevices );
    }

    const PaDeviceInfo *deviceInfo;
    for (int i=0; i < numDevices; i++)  {
        deviceInfo = Pa_GetDeviceInfo( i );
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
    unsigned long framesPerBuffer = paFramesPerBufferUnspecified;
    err = Pa_OpenStream(&stream, &inputParameters, nullptr, sampleRate, framesPerBuffer, paNoFlag,
                        paCallback, &data1);

    std::cout << Pa_GetErrorText(err) << std::endl;

    err = Pa_StartStream(stream);

    std::cout << Pa_GetErrorText(err) << std::endl;

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
    std::cout << Pa_GetErrorText(err) << std::endl;

    return 0;
}
