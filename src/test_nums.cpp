#include "loader.hpp"
#include "model.hpp"
#include <opencv2/opencv.hpp>
#include <iostream>
#include <chrono>

std::vector<float> preprocess(const cv::Mat& img) {
    cv::Mat gray, small;
    if (img.channels() > 1) cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);
    else gray = img.clone();
    cv::resize(gray, small, cv::Size(28, 28));
    std::vector<float> vec(784);
    for (int i = 0; i < 28; ++i)
        for (int j = 0; j < 28; ++j)
            vec[i * 28 + j] = (255.0f - small.at<uchar>(i, j)) / 255.0f;
    return vec;
}

int main() {
    auto model = createModel("mnist-fc");
    for (int d = 0; d <= 9; ++d) {
        std::string path = "nums/" + std::to_string(d) + ".png";
        cv::Mat img = cv::imread(path);
        if (img.empty()) { std::cerr << "Cannot read " << path << std::endl; continue; }
        auto vec = preprocess(img);
        
        auto t0 = std::chrono::high_resolution_clock::now();
        auto probs = model->forward(vec);
        auto t1 = std::chrono::high_resolution_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();
        
        int pred = 0;
        for (int i = 1; i < 10; ++i) if (probs[i] > probs[pred]) pred = i;
        std::cout << "Digit " << d << " -> pred " << pred << " (prob=" << probs[pred] << "), forward=" << ms << " us" << std::endl;
    }
    return 0;
}
