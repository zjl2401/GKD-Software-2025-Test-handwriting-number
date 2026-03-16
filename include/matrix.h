#ifndef MATRIX_H
#define MATRIX_H

#include <vector>
#include <iostream>
#include <stdexcept>
#include <iomanip>
#include <thread>
#include <algorithm>

template<typename T = float>
class Matrix {
public:
    Matrix() : rows_(0), cols_(0) {}

    Matrix(int rows, int cols) : rows_(rows), cols_(cols) {
        data_.resize(static_cast<size_t>(rows_) * cols_, T(0));
    }

    Matrix(int rows, int cols, T value) : rows_(rows), cols_(cols) {
        data_.resize(static_cast<size_t>(rows_) * cols_, value);
    }

    Matrix(const Matrix& other) : rows_(other.rows_), cols_(other.cols_), data_(other.data_) {}

    ~Matrix() = default;

    Matrix& operator=(const Matrix& other) {
        if (this != &other) {
            rows_ = other.rows_;
            cols_ = other.cols_;
            data_ = other.data_;
        }
        return *this;
    }

    Matrix operator+(const Matrix& other) const {
        if (rows_ != other.rows_ || cols_ != other.cols_) {
            throw std::runtime_error("Matrix dimensions do not match for addition");
        }
        Matrix result(rows_, cols_);
        for (int i = 0; i < rows_; i++) {
            for (int j = 0; j < cols_; j++) {
                result.data_[idx(i, j)] = data_[idx(i, j)] + other.data_[other.idx(i, j)];
            }
        }
        return result;
    }

    Matrix operator*(const Matrix& other) const {
        if (cols_ != other.rows_) {
            throw std::runtime_error("Matrix dimensions do not match for multiplication");
        }
        Matrix result(rows_, other.cols_, T(0));
        int nThreads = std::max(1, static_cast<int>(std::thread::hardware_concurrency()));
        if (rows_ >= 4 && nThreads > 1) {
            int chunk = (rows_ + nThreads - 1) / nThreads;
            std::vector<std::thread> threads;
            for (int t = 0; t < nThreads; t++) {
                int iStart = t * chunk;
                int iEnd = std::min(iStart + chunk, rows_);
                if (iStart >= iEnd) break;
                threads.emplace_back([this, &other, &result, iStart, iEnd]() {
                    for (int i = iStart; i < iEnd; i++) {
                        for (int j = 0; j < other.cols_; j++) {
                            T sum = T(0);
                            for (int k = 0; k < cols_; k++) {
                                sum += data_[idx(i, k)] * other.data_[other.idx(k, j)];
                            }
                            result.data_[result.idx(i, j)] = sum;
                        }
                    }
                });
            }
            for (auto& th : threads) th.join();
        } else {
            for (int i = 0; i < rows_; i++) {
                for (int j = 0; j < other.cols_; j++) {
                    T sum = T(0);
                    for (int k = 0; k < cols_; k++) {
                        sum += data_[idx(i, k)] * other.data_[other.idx(k, j)];
                    }
                    result.data_[result.idx(i, j)] = sum;
                }
            }
        }
        return result;
    }

    T& operator()(int row, int col) {
        if (row < 0 || row >= rows_ || col < 0 || col >= cols_) {
            throw std::runtime_error("Matrix index out of bounds");
        }
        return data_[idx(row, col)];
    }

    const T& operator()(int row, int col) const {
        if (row < 0 || row >= rows_ || col < 0 || col >= cols_) {
            throw std::runtime_error("Matrix index out of bounds");
        }
        return data_[idx(row, col)];
    }

    int getRows() const { return rows_; }
    int getCols() const { return cols_; }

    void setValue(int row, int col, T value) {
        if (row < 0 || row >= rows_ || col < 0 || col >= cols_) {
            throw std::runtime_error("Matrix index out of bounds");
        }
        data_[idx(row, col)] = value;
    }

    T getValue(int row, int col) const {
        if (row < 0 || row >= rows_ || col < 0 || col >= cols_) {
            throw std::runtime_error("Matrix index out of bounds");
        }
        return data_[idx(row, col)];
    }

    void fromVector(const std::vector<T>& vec) {
        if (rows_ == 1) {
            if (vec.size() != static_cast<size_t>(cols_)) {
                throw std::runtime_error("Vector size does not match matrix columns");
            }
            for (int j = 0; j < cols_; j++) data_[idx(0, j)] = vec[j];
        } else if (cols_ == 1) {
            if (vec.size() != static_cast<size_t>(rows_)) {
                throw std::runtime_error("Vector size does not match matrix rows");
            }
            for (int i = 0; i < rows_; i++) data_[idx(i, 0)] = vec[i];
        } else {
            throw std::runtime_error("Matrix is not a vector");
        }
    }

    std::vector<T> toVector() const {
        std::vector<T> vec;
        if (rows_ == 1) {
            vec.reserve(static_cast<size_t>(cols_));
            for (int j = 0; j < cols_; j++) vec.push_back(data_[idx(0, j)]);
        } else if (cols_ == 1) {
            vec.reserve(static_cast<size_t>(rows_));
            for (int i = 0; i < rows_; i++) vec.push_back(data_[idx(i, 0)]);
        } else {
            throw std::runtime_error("Matrix is not a vector");
        }
        return vec;
    }

    void print() const {
        for (int i = 0; i < rows_; i++) {
            for (int j = 0; j < cols_; j++) {
                std::cout << std::fixed << std::setprecision(4) << data_[idx(i, j)] << " ";
            }
            std::cout << std::endl;
        }
    }

private:
    int rows_;
    int cols_;
    std::vector<T> data_;

    inline size_t idx(int row, int col) const {
        return static_cast<size_t>(row) * cols_ + col;
    }
};

#endif // MATRIX_H
