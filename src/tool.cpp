#include "tools.hpp"

float percentile(const cv::Mat& x, float p){
    CV_Assert(x.type() == CV_32FC1);

    std::vector<float> values;
    if (x.isContinuous()) {
        values.assign((float*)x.datastart, (float*)x.dataend);
    } else {
        for (int i = 0; i < x.rows; ++i) {
            const float* row_ptr = x.ptr<float>(i);
            values.insert(values.end(), row_ptr, row_ptr + x.cols);
        }
    }

    if (values.empty()) return 0.0f;

    std::sort(values.begin(), values.end());

    float idx = (values.size() - 1) * (p / 100.0f);
    size_t idx_below = static_cast<size_t>(std::floor(idx));
    size_t idx_above = std::min(idx_below + 1, values.size() - 1);

    float weight = idx - idx_below;
    return values[idx_below] * (1.0f - weight) + values[idx_above] * weight;
}

cv::Mat centering(const cv::Mat& x) {
    cv::Scalar mean = cv::mean(x);
    cv::Mat centered;
    cv::absdiff(x, mean[0], centered);
    return centered;
}