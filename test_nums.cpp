#include "include/matrix.h"
#include "include/file_reader.h"
#include "include/model.h"
#include <opencv2/opencv.hpp>
#include <iostream>
#include <iomanip>
#include <filesystem>

Matrix preprocessImage(const cv::Mat& img) {
    cv::Mat resized;
    cv::resize(img, resized, cv::Size(28, 28));
    resized.convertTo(resized, CV_32F, 1.0 / 255.0);
    
    // 倒置：MNIST期望黑色背景(0)和白色笔画(1)
    cv::Mat inverted = 1.0f - resized;
    
    Matrix input(1, 784);
    for (int i = 0; i < 28; i++) {
        for (int j = 0; j < 28; j++) {
            float value = inverted.at<float>(i, j);
            input(0, i * 28 + j) = value;
        }
    }
    
    return input;
}

int main() {
    Model model = ModelLoader::loadModel("mnist-fc");
    
    std::cout << "Testing standard digit images:\n";
    std::cout << "================================\n\n";
    
    for (int digit = 0; digit < 10; digit++) {
        std::string filename = "nums/" + std::to_string(digit) + ".png";
        cv::Mat img = cv::imread(filename, cv::IMREAD_GRAYSCALE);
        
        if (img.empty()) {
            std::cerr << "Failed to load " << filename << "\n";
            continue;
        }
        
        Matrix input = preprocessImage(img);
        std::vector<float> probs = model.forward(input);
        
        int prediction = std::distance(probs.begin(), std::max_element(probs.begin(), probs.end()));
        float confidence = *std::max_element(probs.begin(), probs.end());
        
        std::cout << "Image: " << digit << " -> Predicted: " << prediction 
                  << " (confidence: " << std::fixed << std::setprecision(3) << confidence * 100 << "%)\n";
        std::cout << "  Probabilities: ";
        for (size_t i = 0; i < probs.size(); i++) {
            if (probs[i] > 0.05) {
                std::cout << i << ":" << std::fixed << std::setprecision(3) << probs[i] << " ";
            }
        }
        std::cout << "\n\n";
    }
    
    return 0;
}
