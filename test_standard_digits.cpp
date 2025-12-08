#include "include/file_reader.h"
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cmath>

Matrix preprocessImage(const cv::Mat& img) {
    // 缩小到28x28
    cv::Mat resized;
    cv::resize(img, resized, cv::Size(28, 28));
    
    // 转换为浮点数并归一化到0-1
    resized.convertTo(resized, CV_32F, 1.0 / 255.0);
    
    // 拍扁成一维向量
    Matrix input(1, 784);
    
    for (int i = 0; i < 28; i++) {
        for (int j = 0; j < 28; j++) {
            float value = resized.at<float>(i, j);
            // 根据说明：白底黑字，且是0~1浮点数
            // 白底 = 1.0, 黑字 = 0.0
            // MNIST期望：笔画 = 1.0, 背景 = 0.0
            // 所以需要倒置
            float inverted = 1.0f - value;
            input(0, i * 28 + j) = inverted;
        }
    }
    
    return input;
}

int main() {
    try {
        std::cout << "=== Testing with standard digit images ===" << std::endl;
        
        Model model = ModelLoader::loadModel("mnist-fc");
        std::cout << "Model loaded successfully!" << std::endl << std::endl;
        
        int correctCount = 0;
        
        // 测试0-9的标准数字图片
        for (int digit = 0; digit < 10; digit++) {
            std::string imagePath = "nums/" + std::to_string(digit) + ".png";
            cv::Mat img = cv::imread(imagePath, cv::IMREAD_GRAYSCALE);
            
            if (img.empty()) {
                std::cerr << "Failed to load image: " << imagePath << std::endl;
                continue;
            }
            
            // 预处理
            Matrix input = preprocessImage(img);
            
            // 推理
            std::vector<float> probabilities = model.forward(input);
            
            // 找到概率最大的数字
            auto maxIt = std::max_element(probabilities.begin(), probabilities.end());
            int prediction = static_cast<int>(std::distance(probabilities.begin(), maxIt));
            float confidence = *maxIt;
            
            bool correct = (prediction == digit);
            if (correct) correctCount++;
            
            std::cout << "Image " << digit << ": Predicted=" << prediction 
                     << ", Confidence=" << std::fixed << std::setprecision(4) << confidence;
            
            if (correct) {
                std::cout << " ✓ CORRECT";
            } else {
                std::cout << " ✗ WRONG";
            }
            std::cout << std::endl;
            
            // 打印概率分布
            std::cout << "  Probabilities: ";
            for (int i = 0; i < 10; i++) {
                std::cout << i << ":" << std::setprecision(3) << probabilities[i] << " ";
            }
            std::cout << std::endl << std::endl;
        }
        
        std::cout << "=== Summary ===" << std::endl;
        std::cout << "Correct: " << correctCount << "/10" << std::endl;
        std::cout << "Accuracy: " << (correctCount * 10) << "%" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
