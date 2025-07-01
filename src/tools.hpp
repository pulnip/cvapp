#include <opencv2/opencv.hpp>
#ifndef NO_ANDROID
#include <android/log.h>

#define LOG_TAG "NativeDebug"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#endif

float percentile(const cv::Mat& x, float p);
cv::Mat centering(const cv::Mat& x);