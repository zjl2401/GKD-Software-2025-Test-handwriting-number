#ifdef _WIN32
#define NOMINMAX
#endif
#include "../include/file_reader.h"
#include "../include/preprocess.h"
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>

int main() {
    std::string modelPath = findModelPath("mnist-fc");
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
        std::vector<float> probs = model->forward(preprocessImage(img));
        int pred = 0;
        for (int i = 1; i < 10; i++) if (probs[i] > probs[pred]) pred = i;
        if (pred == d) ok++;
        std::cout << d << " -> " << pred << (pred == d ? " OK" : " FAIL") << "\n";
    }
    std::cout << ok << "/10 OK\n";
    return ok == 10 ? 0 : 1;
}
