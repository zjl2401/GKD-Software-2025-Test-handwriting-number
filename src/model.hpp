#pragma once

#include "matrix.hpp"
#include "utils.hpp"
#include <memory>
#include <string>

class ModelBase {
public:
    virtual ~ModelBase() = default;
    virtual std::vector<float> forward(const std::vector<float>& input) = 0;
};

template<typename T = float>
class Model : public ModelBase {
public:
    Model(const Matrix<T>& w1, const Matrix<T>& b1,
          const Matrix<T>& w2, const Matrix<T>& b2)
        : weight1_(w1), bias1_(b1), weight2_(w2), bias2_(b2) {}
    
    std::vector<float> forward(const std::vector<float>& input) override {
        if (input.size() != 784)
            throw std::runtime_error("Input must be 784 elements");
        Matrix<T> x(1, 784);
        for (size_t i = 0; i < 784; ++i) x(0, i) = static_cast<T>(input[i]);
        
        auto t = x * weight1_;
        t = t + bias1_;
        t = relu(t);
        t = t * weight2_;
        t = t + bias2_;
        
        auto probs = softmax(t);
        std::vector<float> result(probs.size());
        for (size_t i = 0; i < probs.size(); ++i)
            result[i] = static_cast<float>(probs[i]);
        return result;
    }
    
private:
    Matrix<T> weight1_, bias1_, weight2_, bias2_;
};
