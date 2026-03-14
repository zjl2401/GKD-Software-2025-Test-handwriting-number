#include "include/matrix.h"
#include "include/file_reader.h"
#include <iostream>
#include <iomanip>

int main() {
    // 加载偏差
    std::cout << "fc1.bias values:\n";
    Matrix b1 = readMatrixFromBinary("mnist-fc/fc1.bias", 1, 500);
    float sum_b1 = 0;
    for (int j = 0; j < 500; j++) {
        sum_b1 += b1(0, j);
    }
    std::cout << "Sum of fc1.bias: " << std::fixed << std::setprecision(6) << sum_b1 << "\n";
    std::cout << "Mean of fc1.bias: " << sum_b1 / 500.0 << "\n\n";
    
    // 加载fc2.weight
    std::cout << "fc2.weight statistics:\n";
    Matrix w2 = readMatrixFromBinary("mnist-fc/fc2.weight", 10, 500);
    
    for (int i = 0; i < 10; i++) {
        float row_sum = 0, row_mean = 0;
        for (int j = 0; j < 500; j++) {
            row_sum += w2(i, j);
        }
        row_mean = row_sum / 500.0;
        std::cout << "Output " << i << ": sum=" << std::fixed << std::setprecision(6) << row_sum 
                  << " mean=" << row_mean << "\n";
    }
    
    // 加载fc2.bias
    std::cout << "\nfc2.bias values:\n";
    Matrix b2 = readMatrixFromBinary("mnist-fc/fc2.bias", 1, 10);
    for (int j = 0; j < 10; j++) {
        std::cout << "b2[" << j << "] = " << std::fixed << std::setprecision(6) << b2(0, j) << "\n";
    }
    
    return 0;
}
