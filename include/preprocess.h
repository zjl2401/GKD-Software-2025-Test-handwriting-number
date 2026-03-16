#ifndef PREPROCESS_H
#define PREPROCESS_H

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <vector>
#include <algorithm>
#include <cmath>

namespace {

// 根据图像矩估计倾斜角（弧度），用于去倾斜；若像素太少则返回 0
inline double getDeskewAngleRad(const cv::Mat& binaryDigit) {
    cv::Moments m = cv::moments(binaryDigit, true);
    if (m.m00 < 50) return 0.0;
    double denom = m.mu20 - m.mu02;
    if (std::abs(denom) < 1e-6) return 0.0;
    return 0.5 * std::atan2(2.0 * m.mu11, denom);
}

// 对裁切出的数字区域做去倾斜（旋转校正），便于识别倾斜/旋转书写
inline void deskewRoi(cv::Mat& roi) {
    cv::Mat binary;
    cv::threshold(roi, binary, 250, 255, cv::THRESH_BINARY_INV);
    if (cv::countNonZero(binary) < 50) return;
    double angleRad = getDeskewAngleRad(binary);
    double angleDeg = angleRad * 180.0 / CV_PI;
    const double maxCorrectionDeg = 25.0;
    angleDeg = std::max(-maxCorrectionDeg, std::min(maxCorrectionDeg, angleDeg));
    if (std::abs(angleDeg) < 1.0) return;
    cv::Point2f center(roi.cols * 0.5f, roi.rows * 0.5f);
    cv::Mat M = cv::getRotationMatrix2D(center, -angleDeg, 1.0);
    cv::Mat rotated;
    cv::warpAffine(roi, rotated, M, roi.size(), cv::INTER_LINEAR, cv::BORDER_CONSTANT, cv::Scalar(255));
    roi = rotated;
}

} // namespace

// 将图像预处理为 MNIST 输入：转灰度、裁切数字区域、去倾斜、等比缩放到 20 内、居中到 28x28、归一化到 [0,1]，返回 784 维向量
inline std::vector<float> preprocessImage(const cv::Mat& img) {
    cv::Mat work;
    if (img.channels() > 1) {
        cv::cvtColor(img, work, cv::COLOR_BGR2GRAY);
    } else {
        work = img.clone();
    }

    int x1 = work.cols, y1 = work.rows, x2 = 0, y2 = 0;
    for (int i = 0; i < work.rows; i++) {
        const uchar* row = work.ptr<uchar>(i);
        for (int j = 0; j < work.cols; j++) {
            if (row[j] < 250) {
                if (j < x1) x1 = j;
                if (j > x2) x2 = j;
                if (i < y1) y1 = i;
                if (i > y2) y2 = i;
            }
        }
    }
    cv::Mat roi;
    if (x2 >= x1 && y2 >= y1) {
        int pad = 10;
        x1 = std::max(0, x1 - pad);
        y1 = std::max(0, y1 - pad);
        x2 = std::min(work.cols - 1, x2 + pad);
        y2 = std::min(work.rows - 1, y2 + pad);
        roi = work(cv::Rect(x1, y1, x2 - x1 + 1, y2 - y1 + 1)).clone();
    } else {
        roi = work;
    }
    deskewRoi(roi);
    int rw = roi.cols, rh = roi.rows;
    if (rw <= 0) rw = 1;
    if (rh <= 0) rh = 1;
    const double targetSize = 20.0;
    double scale = std::min(targetSize / rw, targetSize / rh);
    int sw = static_cast<int>(rw * scale), sh = static_cast<int>(rh * scale);
    if (sw <= 0) sw = 1;
    if (sh <= 0) sh = 1;
    cv::Mat resized;
    cv::resize(roi, resized, cv::Size(sw, sh), 0, 0,
              (sw < roi.cols || sh < roi.rows) ? cv::INTER_AREA : cv::INTER_LINEAR);
    cv::Mat out28(28, 28, CV_8UC1, cv::Scalar(255));
    int ox = (28 - resized.cols) / 2, oy = (28 - resized.rows) / 2;
    ox = std::max(0, std::min(ox, 28 - resized.cols));
    oy = std::max(0, std::min(oy, 28 - resized.rows));
    resized.copyTo(out28(cv::Rect(ox, oy, resized.cols, resized.rows)));
    out28.convertTo(out28, CV_32F, 1.0 / 255.0);
    std::vector<float> vec(784);
    for (int i = 0; i < 28; i++)
        for (int j = 0; j < 28; j++)
            vec[i * 28 + j] = out28.at<float>(i, j);
    return vec;
}

#endif // PREPROCESS_H
