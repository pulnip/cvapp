cmake_minimum_required(VERSION 3.10)
project(docscanner)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(OpenCV_DIR "${CMAKE_CURRENT_SOURCE_DIR}/opencv/sdk/native/jni")
find_package(OpenCV REQUIRED)

add_library(docscanner SHARED
    native_process.cpp
)

target_include_directories(docscanner PRIVATE
    ${OpenCV_INCLUDE_DIRS}
)
target_link_libraries(docscanner
    ${OpenCV_LIBS}
    log
)
