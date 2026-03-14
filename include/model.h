#ifndef MODEL_H
#define MODEL_H

#include "matrix.h"
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <memory>

// RELU 模板函数
template<typename T>
inline Matrix<T> relu(const Matrix<T>& input) {
    Matrix<T> result(input.getRows(), input.getCols());
    for (int i = 0; i < input.getRows(); i++) {
        for (int j = 0; j < input.getCols(); j++) {
            result(i, j) = std::max(T(0), input(i, j));
        }
    }
    return result;
}

// SoftMax 模板函数
template<typename T>
inline std::vector<T> softmax(const std::vector<T>& input) {
    std::vector<T> result(input.size());
    T max_val = *std::max_element(input.begin(), input.end());
    T sum = T(0);
    for (size_t i = 0; i < input.size(); i++) {
        result[i] = std::exp(input[i] - max_val);
        sum += result[i];
    }
    for (size_t i = 0; i < result.size(); i++) {
        result[i] /= sum;
    }
    return result;
}

// Model 基类，用于多态
class ModelBase {
public:
    virtual ~ModelBase() = default;
    // 输入 784 维 float 向量，返回 10 维 float 概率向量
    virtual std::vector<float> forward(const std::vector<float>& input) = 0;
};

// 模板 Model 类，继承 ModelBase
template<typename T>
class Model : public ModelBase {
public:
    Model(const Matrix<T>& weight1, const Matrix<T>& bias1,
          const Matrix<T>& weight2, const Matrix<T>& bias2)
        : weight1_(weight1), bias1_(bias1), weight2_(weight2), bias2_(bias2) {}

    std::vector<float> forward(const std::vector<float>& input) override {
        int in_features = weight1_.getCols();
        int hidden = weight1_.getRows();
        int out_features = weight2_.getRows();
        
        if (input.size() != static_cast<size_t>(in_features)) {
            throw std::runtime_error("Input size must be " + std::to_string(in_features));
        }
        
        // 将 float 输入转为 Matrix<T>
        Matrix<T> inputMat(1, in_features);
        for (int j = 0; j < in_features; j++) {
            inputMat(0, j) = static_cast<T>(input[j]);
        }
        
        // input (1×in) @ weight1_.T
        Matrix<T> weight1_T(in_features, hidden);
        for (int i = 0; i < in_features; i++) {
            for (int j = 0; j < hidden; j++) {
                weight1_T(i, j) = weight1_(j, i);
            }
        }
        Matrix<T> temp1 = inputMat * weight1_T;
        Matrix<T> temp2 = temp1 + bias1_;
        Matrix<T> temp3 = relu(temp2);
        
        Matrix<T> weight2_T(hidden, out_features);
        for (int i = 0; i < hidden; i++) {
            for (int j = 0; j < out_features; j++) {
                weight2_T(i, j) = weight2_(j, i);
            }
        }
        Matrix<T> temp4 = temp3 * weight2_T;
        Matrix<T> temp5 = temp4 + bias2_;
        
        std::vector<T> outVec = temp5.toVector();
        std::vector<T> probs = softmax(outVec);
        
        // 转回 float 返回
        std::vector<float> result(probs.size());
        for (size_t i = 0; i < probs.size(); i++) {
            result[i] = static_cast<float>(probs[i]);
        }
        return result;
    }

private:
    Matrix<T> weight1_;
    Matrix<T> bias1_;
    Matrix<T> weight2_;
    Matrix<T> bias2_;
};

#endif // MODEL_H
