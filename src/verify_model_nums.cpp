// 用 nums/0.png~9.png 验证模型与预处理是否一致（part6：28x28，/255，不取反）
#ifdef _WIN32
#define NOMINMAX
#endif
#include "../include/file_reader.h"
#include "../include/model.h"
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <fstream>

static std::vector<float> preprocess(const cv::Mat& img) {
    cv::Mat gray, small;
    if (img.channels() > 1) cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);
    else gray = img.clone();
    cv::resize(gray, small, cv::Size(28, 28));
    std::vector<float> vec(784);
    for (int i = 0; i < 28; i++)
        for (int j = 0; j < 28; j++)
            vec[i * 28 + j] = small.at<uchar>(i, j) / 255.0f;
    return vec;
}

int main() {
    std::vector<std::string> modelPaths = { "mnist-fc", "../mnist-fc", "../../mnist-fc" };
    std::string modelPath;
    for (const auto& path : modelPaths) {
        std::ifstream f(path + "/fc1.weight", std::ios::binary);
        if (f.good()) { modelPath = path; break; }
    }
    if (modelPath.empty()) {
        std::cerr << "Cannot find mnist-fc\n";
        return 1;
    }
    std::unique_ptr<ModelBase> model = createModel(modelPath);

    std::vector<std::string> numsBases = { "nums", "../nums", "../../nums" };
    std::string numsBase;
    for (const auto& base : numsBases) {
        cv::Mat t = cv::imread(base + "/0.png");
        if (!t.empty()) { numsBase = base; break; }
    }
    if (numsBase.empty()) {
        std::cerr << "Cannot find nums/0.png\n";
        return 1;
    }

    int ok = 0;
    for (int d = 0; d <= 9; d++) {
        cv::Mat img = cv::imread(numsBase + "/" + std::to_string(d) + ".png");
        if (img.empty()) continue;
        std::vector<float> probs = model->forward(preprocess(img));
        int pred = 0;
        for (int i = 1; i < 10; i++) if (probs[i] > probs[pred]) pred = i;
        if (pred == d) ok++;
        std::cout << d << " -> " << pred << (pred == d ? " OK" : " FAIL") << "\n";
    }
    std::cout << ok << "/10 OK\n";
    return ok == 10 ? 0 : 1;
}
