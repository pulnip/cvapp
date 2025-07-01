#include <cstdlib>
#include "docscan.hpp"
#include "tools.hpp"
#ifndef NO_ANDROID
extern "C" {

int native_entry(uint8_t *input, uint8_t *output,
    int width, int height
){
    try {
        cv::Mat src(height, width, CV_8UC3, input);
        cv::Mat dst;
        lum_doc(src, dst);

        memcpy(output, dst.data, width * height);
        LOGD("lum_doc Success");
        return 0;
    } catch (...) {
        LOGE("lum_doc Error");
        return -1;
    }
}

} // extern "C"
#endif
int main(int argc, char* argv[]){
    cv::Mat src = cv::imread("frame.jpg"), dst;
    lum_doc(src, dst);
    cv::imshow("result", dst);

    return 0;
}