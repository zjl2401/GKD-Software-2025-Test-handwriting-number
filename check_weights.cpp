#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>
#include <cmath>

float readOneFloat(std::ifstream& file) {
    float value;
    file.read(reinterpret_cast<char*>(&value), sizeof(float));
    return value;
}

int main() {
    std::string basePath = "mnist-fc/";
    
    // 检查fc1.weight - 500*784 = 392000个float
    std::ifstream w1(basePath + "fc1.weight", std::ios::binary);
    if (!w1.is_open()) {
        std::cerr << "Cannot open fc1.weight\n";
        return 1;
    }
    
    // 计算文件大小
    w1.seekg(0, std::ios::end);
    long size = w1.tellg();
    w1.seekg(0, std::ios::beg);
    
    std::cout << "fc1.weight file size: " << size << " bytes\n";
    std::cout << "Expected: " << 500 * 784 * 4 << " bytes\n";
    std::cout << "Num floats: " << size / 4 << "\n\n";
    
    // 读取前10个和后10个值
    std::cout << "First 10 values: ";
    for (int i = 0; i < 10; i++) {
        float val = readOneFloat(w1);
        std::cout << std::fixed << std::setprecision(6) << val << " ";
    }
    std::cout << "\n";
    
    // 跳到最后10个
    w1.seekg(size - 10 * 4, std::ios::beg);
    std::cout << "Last 10 values: ";
    for (int i = 0; i < 10; i++) {
        float val = readOneFloat(w1);
        std::cout << std::fixed << std::setprecision(6) << val << " ";
    }
    std::cout << "\n\n";
    
    // 计算统计信息
    w1.seekg(0, std::ios::beg);
    float minVal = 1e9, maxVal = -1e9, sumVal = 0.0;
    int count = size / 4;
    
    for (int i = 0; i < count; i++) {
        float val = readOneFloat(w1);
        minVal = std::min(minVal, val);
        maxVal = std::max(maxVal, val);
        sumVal += val;
    }
    
    std::cout << "fc1.weight statistics:\n";
    std::cout << "  Min: " << std::fixed << std::setprecision(6) << minVal << "\n";
    std::cout << "  Max: " << std::fixed << std::setprecision(6) << maxVal << "\n";
    std::cout << "  Mean: " << std::fixed << std::setprecision(6) << sumVal / count << "\n";
    
    w1.close();
    
    // 检查fc1.bias - 1*500 = 500个float
    std::ifstream b1(basePath + "fc1.bias", std::ios::binary);
    b1.seekg(0, std::ios::end);
    size = b1.tellg();
    b1.seekg(0, std::ios::beg);
    
    std::cout << "\nfc1.bias file size: " << size << " bytes\n";
    std::cout << "Expected: " << 500 * 4 << " bytes\n\n";
    
    // 检查fc2.weight - 10*500 = 5000个float
    std::ifstream w2(basePath + "fc2.weight", std::ios::binary);
    w2.seekg(0, std::ios::end);
    size = w2.tellg();
    w2.seekg(0, std::ios::beg);
    
    std::cout << "fc2.weight file size: " << size << " bytes\n";
    std::cout << "Expected: " << 10 * 500 * 4 << " bytes\n";
    std::cout << "Num floats: " << size / 4 << "\n";
    
    // 读fc2.weight的所有10*500值并计算每行的和（对应10个输出）
    std::cout << "\nfc2.weight row sums (should be 10 values, one per output):\n";
    
    float rowSums[10] = {0};
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 500; j++) {
            float val = readOneFloat(w2);
            rowSums[i] += val;
        }
        std::cout << "  Output " << i << ": sum=" << std::fixed << std::setprecision(6) << rowSums[i] << "\n";
    }
    
    w2.close();
    
    return 0;
}
