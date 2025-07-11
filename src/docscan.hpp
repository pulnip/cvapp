#include <tuple>
#include <stdint.h>
#include <opencv2/opencv.hpp>

int lum_doc(const cv::Mat& src, cv::Mat& dst);

struct InitialMask{
    cv::Mat L, a, b;
    cv::Mat bright, neutral, saturation;
    cv::Mat combination;
};

struct Quad{
    cv::Point2f tl;
    cv::Point2f bl;
    cv::Point2f br;
    cv::Point2f tr;
};

cv::Mat lum_roi(const cv::Mat& frame);
InitialMask initialMask(const cv::Mat& frame);
cv::Mat refineMask(const InitialMask& mask);
std::tuple<cv::Mat, cv::Mat, cv::Mat> candidatePixels(const InitialMask& mask);
int numHistPeak(const cv::Mat& flat);
cv::Mat clusterPixel(const cv::Mat& flat, int, const cv::Mat& mask);
cv::Mat cleanupSmallBlob(const cv::Mat& mask);
cv::Mat extractLargestBlob(const cv::Mat& mask);

float maskRectangularity(cv::Mat mask);
Quad toQuad(const std::vector<cv::Point2f>& points);
float quadSize(const Quad& quad);

// return seperator indexes.
cv::Mat assumePageSeperator(const cv::Mat& rectangularities);