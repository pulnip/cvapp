#include <cstdlib>
#include "docscan.hpp"
#include "tools.hpp"

#ifndef NO_ANDROID
extern "C" {

int lum_doc(uint8_t *input, uint8_t *output,
    int width, int height
){
    try{
        cv::Mat src(height, width, CV_8UC3, input);
        cv::Mat dst;
        lum_doc_impl(src, dst);
        cv::resize(dst, dst, cv::Size(src.cols, src.rows));
        cv::cvtColor(dst, dst, cv::COLOR_GRAY2BGR);

        memcpy(output, dst.data, 3 * width * height);
        LOGD("lum_doc Success");
        return 0;
    } catch (...){
        LOGE("lum_doc Error");
        return -1;
    }
}

} // extern "C"
#endif
int main(int argc, char* argv[]){
    cv::Mat src = cv::imread("frame.jpg"), dst;
    lum_doc_impl(src, dst);
    cv::imshow("result", dst);

    return 0;
}