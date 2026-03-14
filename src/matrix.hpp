#pragma once

#include <vector>
#include <stdexcept>
#include <thread>
#include <algorithm>

template<typename T = float>
class Matrix {
public:
    Matrix() : rows_(0), cols_(0) {}
    
    Matrix(size_t rows, size_t cols) : rows_(rows), cols_(cols) {
        data_.resize(rows * cols, T(0));
    }
    
    Matrix(size_t rows, size_t cols, const std::vector<T>& data)
        : rows_(rows), cols_(cols), data_(data) {}
    
    T& operator()(size_t i, size_t j) { return data_[i * cols_ + j]; }
    const T& operator()(size_t i, size_t j) const { return data_[i * cols_ + j]; }
    
    size_t rows() const { return rows_; }
    size_t cols() const { return cols_; }
    
    Matrix<T> operator*(const Matrix<T>& other) const {
        if (cols_ != other.rows_)
            throw std::runtime_error("Matrix dimensions mismatch for multiplication");
        Matrix<T> result(rows_, other.cols_);
        unsigned int nThreads = std::thread::hardware_concurrency();
        if (nThreads == 0) nThreads = 1;
        size_t chunk = (rows_ + nThreads - 1) / nThreads;
        std::vector<std::thread> threads;
        for (unsigned int t = 0; t < nThreads; ++t) {
            size_t iStart = t * chunk, iEnd = std::min((t + 1) * chunk, rows_);
            if (iStart >= iEnd) break;
            threads.emplace_back([this, &other, &result, iStart, iEnd]() {
                for (size_t i = iStart; i < iEnd; ++i) {
                    for (size_t k = 0; k < cols_; ++k) {
                        T a = (*this)(i, k);
                        for (size_t j = 0; j < other.cols_; ++j)
                            result(i, j) += a * other(k, j);
                    }
                }
            });
        }
        for (auto& th : threads) th.join();
        return result;
    }
    
    Matrix<T> operator+(const Matrix<T>& other) const {
        if (rows_ != other.rows_ || cols_ != other.cols_)
            throw std::runtime_error("Matrix dimensions mismatch for addition");
        Matrix<T> result(rows_, cols_);
        for (size_t i = 0; i < data_.size(); ++i)
            result.data_[i] = data_[i] + other.data_[i];
        return result;
    }
    
    T* data() { return data_.data(); }
    const T* data() const { return data_.data(); }
    
private:
    size_t rows_, cols_;
    std::vector<T> data_;
};
