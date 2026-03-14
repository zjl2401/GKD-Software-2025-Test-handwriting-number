#ifndef MODEL_H
#define MODEL_H

#include "matrix.h"
#include <vector>
#include <string>
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
    
    // forward方法：输入 1×in_features 矩阵，返回长为 10 的向量
    std::vector<float> forward(const Matrix& input) {
        int in_features = weight1_.getCols();  // 784 或由 meta.json 决定
        int hidden = weight1_.getRows();
        int out_features = weight2_.getRows();  // 10
        
        if (input.getRows() != 1 || input.getCols() != in_features) {
            throw std::runtime_error("Input matrix must be 1 * " + std::to_string(in_features));
        }
        
        // input (1×in) @ weight1_.T (in×hidden) = temp1 (1×hidden)
        Matrix weight1_T(in_features, hidden);
        for (int i = 0; i < in_features; i++) {
            for (int j = 0; j < hidden; j++) {
                weight1_T(i, j) = weight1_(j, i);
            }
        }
        Matrix temp1 = input * weight1_T;
        
        Matrix temp2 = temp1 + bias1_;
        Matrix temp3 = relu(temp2);
        
        // temp3 (1×hidden) @ weight2_.T (hidden×out) = temp4 (1×out)
        Matrix weight2_T(hidden, out_features);
        for (int i = 0; i < hidden; i++) {
            for (int j = 0; j < out_features; j++) {
                weight2_T(i, j) = weight2_(j, i);
            }
        }
        Matrix temp4 = temp3 * weight2_T;
        
        Matrix temp5 = temp4 + bias2_;
        
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
