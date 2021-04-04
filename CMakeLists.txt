cmake_minimum_required(VERSION 3.15)
project(music_visualizer)

set(CMAKE_CXX_STANDARD 20)

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(music_visualizer main.cpp fft.hpp fft.cpp)
add_subdirectory("/Users/anton/portaudio/portaudio" BINARY)

set(OpenCV_LIBS opencv_core opencv_imgproc opencv_highgui opencv_imgcodecs)

target_link_libraries(music_visualizer ${OpenCV_LIBS})
target_link_libraries(music_visualizer portaudio_static)