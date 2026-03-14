#pragma once

#include "matrix.hpp"
#include <vector>
#include <cmath>
#include <algorithm>

template<typename T>
Matrix<T> relu(const Matrix<T>& m) {
    Matrix<T> result(m.rows(), m.cols());
    for (size_t i = 0; i < m.rows(); ++i) {
        for (size_t j = 0; j < m.cols(); ++j) {
            T val = m(i, j);
            result(i, j) = val > T(0) ? val : T(0);
        }
    }
    return result;
}

template<typename T>
std::vector<T> softmax(const Matrix<T>& vec) {
    size_t n = vec.cols();
    if (vec.rows() == 1) {
        std::vector<T> result(n);
        T maxVal = vec(0, 0);
        for (size_t i = 0; i < n; ++i)
            maxVal = std::max(maxVal, vec(0, i));
        T sum = T(0);
        for (size_t i = 0; i < n; ++i) {
            result[i] = std::exp(vec(0, i) - maxVal);
            sum += result[i];
        }
        for (size_t i = 0; i < n; ++i)
            result[i] /= sum;
        return result;
    }
    std::vector<T> result(n);
    T maxVal = vec(0, 0);
    for (size_t i = 0; i < n; ++i)
        maxVal = std::max(maxVal, vec(i, 0));
    T sum = T(0);
    for (size_t i = 0; i < n; ++i) {
        result[i] = std::exp(vec(i, 0) - maxVal);
        sum += result[i];
    }
    for (size_t i = 0; i < n; ++i)
        result[i] /= sum;
    return result;
}
