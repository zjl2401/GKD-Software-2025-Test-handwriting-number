#include "include/file_reader.h"
#include <iostream>
#include <iomanip>
#include <cmath>
#include <algorithm>

int main() {
    try {
        std::cout << "=== Testing mnist-fc model ===" << std::endl;
        Model model = ModelLoader::loadModel("mnist-fc");
        
        std::cout << "Model loaded successfully!" << std::endl;
        
        // 创建一个测试输入：全黑（0）
        Matrix testBlack(1, 784, 0.0f);
        std::vector<float> resultBlack = model.forward(testBlack);
        
        std::cout << "\nTest 1: All black (0.0) input:" << std::endl;
        for (int i = 0; i < 10; i++) {
            std::cout << "  " << i << ": " << std::fixed << std::setprecision(4) << resultBlack[i] << std::endl;
        }
        int predBlack = std::distance(resultBlack.begin(), std::max_element(resultBlack.begin(), resultBlack.end()));
        std::cout << "Prediction: " << predBlack << std::endl;
        
        // 创建一个测试输入：全白（1）
        Matrix testWhite(1, 784, 1.0f);
        std::vector<float> resultWhite = model.forward(testWhite);
        
        std::cout << "\nTest 2: All white (1.0) input:" << std::endl;
        for (int i = 0; i < 10; i++) {
            std::cout << "  " << i << ": " << std::fixed << std::setprecision(4) << resultWhite[i] << std::endl;
        }
        int predWhite = std::distance(resultWhite.begin(), std::max_element(resultWhite.begin(), resultWhite.end()));
        std::cout << "Prediction: " << predWhite << std::endl;
        
        // 创建一个简单的图案：模拟"9"（上面圆形，下面竖线）
        Matrix test9(1, 784, 0.0f);
        // 上面的圆形（行 5-12，列 10-18）
        for (int i = 5; i < 13; i++) {
            for (int j = 10; j < 19; j++) {
                test9(0, i * 28 + j) = 0.5f;
            }
        }
        // 下面的竖线（行 13-20，列 14-16）
        for (int i = 13; i < 21; i++) {
            for (int j = 14; j < 17; j++) {
                test9(0, i * 28 + j) = 1.0f;
            }
        }
        std::vector<float> result9 = model.forward(test9);
        
        std::cout << "\nTest 3: Pattern resembling '9':" << std::endl;
        for (int i = 0; i < 10; i++) {
            std::cout << "  " << i << ": " << std::fixed << std::setprecision(4) << result9[i] << std::endl;
        }
        int pred9 = std::distance(result9.begin(), std::max_element(result9.begin(), result9.end()));
        std::cout << "Prediction: " << pred9 << std::endl;
        
        // 检查权重的统计信息
        std::cout << "\n=== Weight Statistics ===" << std::endl;
        std::cout << "Note: Checking if weights are loaded correctly" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
