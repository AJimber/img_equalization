#include <iostream>
#include "common_code.hpp"
#include <opencv2/imgproc.hpp>

cv::Mat
fsiv_compute_image_histogram(const cv::Mat &in)
{
    CV_Assert(in.type() == CV_8UC1);
    cv::Mat hist;
    std::vector<cv::Mat> images {in};
    std::vector<int> channels = {0};
    std::vector<int> histSize = {256};
    std::vector<float> ranges = {0.0, 256.0f};
    cv::calcHist(images, channels, cv::Mat(), hist, histSize, ranges, false);

    CV_Assert(!hist.empty());
    CV_Assert(hist.type() == CV_32FC1);
    CV_Assert(hist.rows == 256 && hist.cols == 1);
    return hist;
}

void fsiv_normalize_histogram(cv::Mat &hist)
{
    CV_Assert(hist.type() == CV_32FC1);
    CV_Assert(hist.rows == 256 && hist.cols == 1);
    cv::normalize(hist, hist, 1, 0, cv::NORM_L1, -1, cv::noArray());

    CV_Assert(hist.type() == CV_32FC1);
    CV_Assert(hist.rows == 256 && hist.cols == 1);
    CV_Assert(cv::sum(hist)[0] == 0.0 || cv::abs(cv::sum(hist)[0] - 1.0) <= 1.0e-6);
}

void fsiv_accumulate_histogram(cv::Mat &hist)
{
    CV_Assert(hist.type() == CV_32FC1);
    CV_Assert(hist.rows == 256 && hist.cols == 1);
    for(int i = 1; i < 256; i++)
        hist.at<float>(i) += hist.at<float>(i - 1);

    CV_Assert(hist.type() == CV_32FC1);
    CV_Assert(hist.rows == 256 && hist.cols == 1);
}

void fsiv_compute_clipped_histogram(cv::Mat &h, float cl)
{
    CV_Assert(h.type() == CV_32FC1);
    CV_Assert(h.rows == 256 && h.cols == 1);
    float area = 0.0;
    for(int i = 0; i < 256; i++)
    {
        area += std::max(0.0f, h.at<float>(i) - cl);
        h.at<float>(i) = std::min(h.at<float>(i), cl);
    }

    h += area / 256.0f;

    CV_Assert(h.type() == CV_32FC1);
    CV_Assert(h.rows == 256 && h.cols == 1);
}

float fsiv_compute_actual_clipping_histogram_value(const cv::Mat &h, float s)
{
    CV_Assert(h.type() == CV_32FC1);
    CV_Assert(h.rows == 256 && h.cols == 1);

    float CL = s * cv::sum(h)[0] / h.rows;

    int top = CL;
    int bottom = 0;

    while((top - bottom) > 1.0)
    {
        float middle = (top + bottom) / 2;
        float R = 0.0;
        for(int i = 0; i < h.rows; i++)
        {
            if(h.at<float>(i, 0) > middle)
            {
                R += h.at<float>(i, 0) - middle;
            }
        }
        if(R > ((CL - middle) * h.rows))
        {
            top = middle;
        }
        else
        {
            bottom = middle;
        }
    }

    return bottom;
}

cv::Mat
fsiv_create_equalization_lookup_table(const cv::Mat &hist,
                                      float s)
{
    CV_Assert(hist.type() == CV_32FC1);
    CV_Assert(hist.rows == 256 && hist.cols == 1);
    cv::Mat lkt = hist.clone();

    if (s >= 1.0)
    {
        float CL = fsiv_compute_actual_clipping_histogram_value(hist, s);
        fsiv_compute_clipped_histogram(lkt, CL);
    }
    fsiv_normalize_histogram(lkt);
    fsiv_accumulate_histogram(lkt);
    lkt.convertTo(lkt, CV_8U, 255.0, 0.0);

    CV_Assert(lkt.type() == CV_8UC1);
    CV_Assert(lkt.rows == 256 && lkt.cols == 1);
    return lkt;
}

cv::Mat
fsiv_apply_lookup_table(const cv::Mat &in, const cv::Mat &lkt,
                        cv::Mat &out)
{
    CV_Assert(in.type() == CV_8UC1);
    CV_Assert(lkt.type() == CV_8UC1);
    CV_Assert(lkt.rows == 256 && lkt.cols == 1);
    CV_Assert(out.empty() || (out.type() == CV_8UC1 &&
                              out.rows == in.rows && out.cols == in.cols));

    cv::LUT(in, lkt, out);
    CV_Assert(out.rows == in.rows && out.cols == in.cols && out.type() == in.type());
    return out;
}
