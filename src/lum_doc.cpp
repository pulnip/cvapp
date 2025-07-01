#include <algorithm>
#include <vector>
#include <opencv2/opencv.hpp>
#include "tools.hpp"
#include "docscan.hpp"

using namespace std;
using namespace cv;

int lum_doc(const Mat& src, Mat& dst) {
    auto roi = lum_roi(src);
    auto rectangularity = maskRectangularity(roi);
    dst = roi;

#ifndef NO_ANDROID
    LOGD("Rectangularity: %f", rectangularity);
#else
#endif

    return 0;
}

#define BRIGHT_PERC (50)
#define SAT_THR     (40)
#define A_RANGE     (10)
#define B_RANGE     (10)
#define L_MERGE     (80)

Mat lum_roi(const Mat& frame){
    CV_Assert(frame.channels() == 3);

    auto mask = initialMask(frame);
    auto refined = refineMask(mask);
    
    auto region = cleanupSmallBlob(refined) * 255;
    auto blob = extractLargestBlob(region);
    return blob;
}

InitialMask initialMask(const Mat& frame){
    InitialMask mask;
    Mat Lab, hsv;
    cvtColor(frame, Lab, COLOR_BGR2Lab);
    cvtColor(frame, hsv, COLOR_BGR2HSV);

    vector<Mat> planes;
    split(Lab, planes);
    mask.L = planes[0];
    mask.a = planes[1];
    mask.b = planes[2];
    planes.clear();
    split(hsv, planes);

    Mat L_float;
    mask.L.convertTo(L_float, CV_32F);

    auto a_centered = centering(mask.a);
    auto b_centered = centering(mask.b);

    auto bright_threshold = percentile(L_float, BRIGHT_PERC);
    auto a_neutral = a_centered < A_RANGE;
    auto b_neutral = b_centered < A_RANGE;

    // CV_8U
    auto bright = (mask.L > bright_threshold);
    auto neutral = (a_neutral & b_neutral);
    auto saturation = (planes[1] < SAT_THR);
    auto combination = bright & neutral & saturation;

    mask.bright = bright;
    mask.neutral = neutral;
    mask.saturation = saturation;
    mask.combination = combination;

    return mask;
}

Mat refineMask(const InitialMask& mask){
    auto [candidate_mask, vals, L_vals] = candidatePixels(mask);
    auto k = clamp(numHistPeak(L_vals), 2, 4);
    auto refined_mask = clusterPixel(vals, k, candidate_mask);

    if(countNonZero(refined_mask) < 2000){
#ifndef NO_ANDROID
        LOGD("too sparse");
#else
#endif
        refined_mask = mask.combination;
    }
    return refined_mask;
}

tuple<Mat, Mat, Mat> candidatePixels(
    const InitialMask& mask
){
    auto candidate = mask.bright & mask.saturation;

    Mat L_float, a_float, b_float;
    if(mask.L.type() != CV_32F)
        mask.L.convertTo(L_float, CV_32F); else L_float = mask.L;
    if(mask.a.type() != CV_32F)
        mask.a.convertTo(a_float, CV_32F); else a_float = mask.L;
    if(mask.b.type() != CV_32F)
        mask.b.convertTo(b_float, CV_32F); else b_float = mask.L;

    vector<Point> indices;
    findNonZero(candidate, indices);

    cv::Mat vals(indices.size(), 3, CV_32F);
    cv::Mat L_vals(indices.size(), 3, CV_32F);

    for(size_t i=0; i<indices.size(); ++i){
        int y = indices[i].y;
        int x = indices[i].x;
        vals.at<float>(i, 0) = L_float.at<float>(y, x);
        vals.at<float>(i, 1) = a_float.at<float>(y, x);
        vals.at<float>(i, 2) = b_float.at<float>(y, x);
        L_vals.at<float>(i, 0) = L_float.at<float>(y, x);
    }
    return {candidate, vals, L_vals};
}

int numHistPeak(const cv::Mat& flat){
    int histSize = 256;
    float range[] = {0, 256};
    const float* histRange = {range};

    cv::Mat hist;
    calcHist(
        &flat, 1, 0, Mat(), hist, 1,
        &histSize, &histRange
    );
    hist.convertTo(hist, CV_32F);

    cv::Mat hist_blur;
    cv::GaussianBlur(hist, hist_blur, cv::Size(1, 5), 0, 0, cv::BORDER_REPLICATE);

    std::vector<int> peaks;
    double maxVal = 0;
    minMaxLoc(hist_blur, nullptr, &maxVal, nullptr, nullptr);
    auto prom = 0.05f * maxVal;

    for (int i = 1; i < hist_blur.rows - 1; ++i) {
        float prev = hist_blur.at<float>(i - 1);
        float curr = hist_blur.at<float>(i);
        float next = hist_blur.at<float>(i + 1);
        if (prev < curr && curr > next && curr > prom) {
            if (!peaks.empty() && (i - peaks.back() < 8)) {
                if (curr > hist_blur.at<float>(peaks.back())) {
                    peaks.back() = i;
                }
            } else {
                peaks.push_back(i);
            }
        }
    }
    return static_cast<int>(peaks.size());
}

cv::Mat clusterPixel(const cv::Mat& flat, int k, const cv::Mat& mask){
    cv::Mat labels, centers;
    cv::TermCriteria criteria(cv::TermCriteria::EPS + cv::TermCriteria::MAX_ITER, 10, 1.0);
    cv::kmeans(flat, k, labels, criteria, 3, cv::KMEANS_PP_CENTERS, centers);

    cv::Mat L_means = centers.col(0); // (k,1)
    double min_L, top_L;
    cv::minMaxLoc(L_means, &min_L, &top_L);

    std::vector<int> page_idxs;
    for (int i = 0; i < L_means.rows; ++i) {
        if (L_means.at<float>(i, 0) > top_L - L_MERGE)
            page_idxs.push_back(i);
    }

    cv::Mat refined = cv::Mat::zeros(mask.size(), CV_8U);

    int idx = 0;
    for (int y = 0; y < mask.rows; ++y) {
        for (int x = 0; x < mask.cols; ++x) {
            if (mask.at<uchar>(y, x)) {
                int cluster_id = labels.at<int>(idx, 0);
                if (std::find(page_idxs.begin(), page_idxs.end(), cluster_id) != page_idxs.end()) {
                    refined.at<uchar>(y, x) = 1;
                }
                ++idx;
            }
        }
    }
    return refined;
}

cv::Mat cleanupSmallBlob(const cv::Mat& mask){
    cv::Mat k_open  = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5));
    cv::Mat k_dil   = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
    cv::Mat k_close = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(11, 11));

    cv::Mat result;
    cv::morphologyEx(mask, result, cv::MORPH_OPEN, k_open);
    cv::dilate(result, result, k_dil);
    cv::morphologyEx(result, result, cv::MORPH_CLOSE, k_close);
    return result;
}

cv::Mat extractLargestBlob(const cv::Mat& mask){
    cv::Mat label_img, stats, centroids;
    int num = cv::connectedComponentsWithStats(mask, label_img, stats, centroids);

    if (num <= 1) {
        throw std::runtime_error("No document-like region found — adjust thresholds.");
    }

    cv::Mat areas = stats.col(cv::CC_STAT_AREA).rowRange(1, num);
    cv::Point max_loc;
    double max_val;
    cv::minMaxLoc(areas, nullptr, &max_val, nullptr, &max_loc);
    int idx = 1 + max_loc.y;

    cv::Mat result = (label_img == idx);
    result.convertTo(result, CV_8U, 255);

    return result;
}


float maskRectangularity(Mat mask) {
    CV_Assert(mask.channels() == 1);

    vector<vector<Point>> contours;
    findContours(mask, contours,
        RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
    auto rect = minAreaRect(contours[0]);
    vector<Point2f> points(4);
    rect.points(points);
    auto quad = toQuad(points);

    auto mask_size = countNonZero(mask);
    auto quad_size = quadSize(quad);
    
    return mask_size / quad_size;
}

Quad toQuad(const vector<Point2f>& points){
    auto sum = [](const auto& lhs, const auto& rhs){
        return lhs.x + lhs.y < rhs.x + rhs.y;
    };
    auto diff = [](const auto& lhs, const auto& rhs){
        return lhs.x - lhs.y < rhs.x - rhs.y;
    };

    auto tl = *min_element(points.cbegin(), points.cend(), sum);
    auto bl = *max_element(points.cbegin(), points.cend(), diff);
    auto br = *max_element(points.cbegin(), points.cend(), sum);
    auto tr = *min_element(points.cbegin(), points.cend(), diff);

    return {tl, bl, br, tr};
}

static float triangleSize(const Point2f& p1, const Point2f& p2, const Point2f& p3){
    return 0.5 * std::abs(
        (p2.x-p1.x)*(p3.y-p1.y) -
        (p2.y-p1.y)*(p3.x-p1.x));
}

float quadSize(const Quad& quad){
    auto tlSize = triangleSize(quad.tl, quad.bl, quad.tr);
    auto brSize = triangleSize(quad.br, quad.tr, quad.bl);
    return tlSize + brSize;
}