#ifndef MODEL_H
#define MODEL_H

#include "matrix.h"
#include <vector>
#include <cmath>
#include <algorithm>

// RELU函数：对矩阵的每个元素与0取max
inline Matrix relu(const Matrix& input) {
    Matrix result(input.getRows(), input.getCols());
    for (int i = 0; i < input.getRows(); i++) {
        for (int j = 0; j < input.getCols(); j++) {
            result(i, j) = std::max(0.0f, input(i, j));
        }
    }
    return result;
}

// SoftMax函数：输入向量，返回向量
inline std::vector<float> softmax(const std::vector<float>& input) {
    std::vector<float> result(input.size());
    
    // 找到最大值，用于数值稳定性
    float max_val = *std::max_element(input.begin(), input.end());
    
    // 计算exp(x_i - max_val)
    float sum = 0.0f;
    for (size_t i = 0; i < input.size(); i++) {
        result[i] = std::exp(input[i] - max_val);
        sum += result[i];
    }
    
    // 归一化
    for (size_t i = 0; i < result.size(); i++) {
        result[i] /= sum;
    }
    
    return result;
}

class Model {
public:
    // 构造函数：传入四个矩阵
    Model(const Matrix& weight1, const Matrix& bias1, 
          const Matrix& weight2, const Matrix& bias2)
        : weight1_(weight1), bias1_(bias1), weight2_(weight2), bias2_(bias2) {
    }
    
    // forward方法：输入1*784矩阵，返回长为10的向量
    std::vector<float> forward(const Matrix& input) {
        // 输入应该是1 * 784
        if (input.getRows() != 1 || input.getCols() != 784) {
            throw std::runtime_error("Input matrix must be 1 * 784");
        }
        
        // 正确的矩阵乘法过程：
        // input (1×784) @ weight1_.T (784×500) = temp1 (1×500)
        // 但weight1_已经是 [500, 784]，所以需要转置为 [784, 500]
        // 创建转置矩阵
        Matrix weight1_T(784, 500);
        for (int i = 0; i < 784; i++) {
            for (int j = 0; j < 500; j++) {
                weight1_T(i, j) = weight1_(j, i);
            }
        }
        Matrix temp1 = input * weight1_T;
        
        // + b1 -> 1 * 500
        Matrix temp2 = temp1 + bias1_;
        
        // relu -> 1 * 500
        Matrix temp3 = relu(temp2);
        
        // temp3 (1×500) @ weight2_.T (500×10) = temp4 (1×10)
        // weight2_是 [10, 500]，需要转置为 [500, 10]
        Matrix weight2_T(500, 10);
        for (int i = 0; i < 500; i++) {
            for (int j = 0; j < 10; j++) {
                weight2_T(i, j) = weight2_(j, i);
            }
        }
        Matrix temp4 = temp3 * weight2_T;
        
        // + b2 -> 1 * 10
        Matrix temp5 = temp4 + bias2_;
        
        // softmax -> 10维向量
        std::vector<float> output = temp5.toVector();
        return softmax(output);
    }

private:
    Matrix weight1_;  // 500 * 784 (PyTorch格式：[out_features, in_features])
    Matrix bias1_;    // 1 * 500
    Matrix weight2_;  // 10 * 500 (PyTorch格式：[out_features, in_features])
    Matrix bias2_;    // 1 * 10
};

#endif // MODEL_H
