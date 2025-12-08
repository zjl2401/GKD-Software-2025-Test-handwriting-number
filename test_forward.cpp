#include "include/matrix.h"
#include "include/file_reader.h"
#include "include/model.h"
#include <iostream>
#include <iomanip>
#include <cmath>

int main() {
    // 加载模型
    Model model = ModelLoader::loadModel("mnist-fc");
    
    // 测试1：输入全0（黑色背景）
    std::cout << "=== Test 1: All black (0) ===\n";
    Matrix allBlack(1, 784, 0.0f);
    std::vector<float> prob1 = model.forward(allBlack);
    for (size_t i = 0; i < prob1.size(); i++) {
        std::cout << i << ": " << std::fixed << std::setprecision(4) << prob1[i] << "\n";
    }
    
    // 测试2：输入全1（白色背景）
    std::cout << "\n=== Test 2: All white (1) ===\n";
    Matrix allWhite(1, 784, 1.0f);
    std::vector<float> prob2 = model.forward(allWhite);
    for (size_t i = 0; i < prob2.size(); i++) {
        std::cout << i << ": " << std::fixed << std::setprecision(4) << prob2[i] << "\n";
    }
    
    // 测试3：输入全0.5（中灰色）
    std::cout << "\n=== Test 3: All gray (0.5) ===\n";
    Matrix allGray(1, 784, 0.5f);
    std::vector<float> prob3 = model.forward(allGray);
    for (size_t i = 0; i < prob3.size(); i++) {
        std::cout << i << ": " << std::fixed << std::setprecision(4) << prob3[i] << "\n";
    }
    
    // 测试4：只有第一个像素为1，其余为0
    std::cout << "\n=== Test 4: Only first pixel = 1 ===\n";
    Matrix sparse1(1, 784, 0.0f);
    sparse1(0, 0) = 1.0f;
    std::vector<float> prob4 = model.forward(sparse1);
    for (size_t i = 0; i < prob4.size(); i++) {
        std::cout << i << ": " << std::fixed << std::setprecision(4) << prob4[i] << "\n";
    }
    
    // 测试5：只有最后一个像素为1，其余为0
    std::cout << "\n=== Test 5: Only last pixel = 1 ===\n";
    Matrix sparse2(1, 784, 0.0f);
    sparse2(0, 783) = 1.0f;
    std::vector<float> prob5 = model.forward(sparse2);
    for (size_t i = 0; i < prob5.size(); i++) {
        std::cout << i << ": " << std::fixed << std::setprecision(4) << prob5[i] << "\n";
    }
    
    return 0;
}
