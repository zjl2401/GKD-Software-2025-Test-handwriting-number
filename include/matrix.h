#ifndef MATRIX_H
#define MATRIX_H

#include <vector>
#include <iostream>
#include <stdexcept>
#include <iomanip>

template<typename T = float>
class Matrix {
public:
    // 构造函数
    Matrix() : rows_(0), cols_(0) {}
    
    Matrix(int rows, int cols) : rows_(rows), cols_(cols) {
        allocate();
    }
    
    Matrix(int rows, int cols, T value) : rows_(rows), cols_(cols) {
        allocate();
        for (int i = 0; i < rows_; i++) {
            for (int j = 0; j < cols_; j++) {
                data_[i][j] = value;
            }
        }
    }
    
    Matrix(const Matrix& other) : rows_(other.rows_), cols_(other.cols_) {
        allocate();
        for (int i = 0; i < rows_; i++) {
            for (int j = 0; j < cols_; j++) {
                data_[i][j] = other.data_[i][j];
            }
        }
    }
    
    ~Matrix() {}
    
    Matrix& operator=(const Matrix& other) {
        if (this != &other) {
            rows_ = other.rows_;
            cols_ = other.cols_;
            allocate();
            for (int i = 0; i < rows_; i++) {
                for (int j = 0; j < cols_; j++) {
                    data_[i][j] = other.data_[i][j];
                }
            }
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
                result.data_[i][j] = data_[i][j] + other.data_[i][j];
            }
        }
        return result;
    }
    
    Matrix operator*(const Matrix& other) const {
        if (cols_ != other.rows_) {
            throw std::runtime_error("Matrix dimensions do not match for multiplication");
        }
        Matrix result(rows_, other.cols_, T(0));
        for (int i = 0; i < rows_; i++) {
            for (int j = 0; j < other.cols_; j++) {
                for (int k = 0; k < cols_; k++) {
                    result.data_[i][j] += data_[i][k] * other.data_[k][j];
                }
            }
        }
        return result;
    }
    
    T& operator()(int row, int col) {
        if (row < 0 || row >= rows_ || col < 0 || col >= cols_) {
            throw std::runtime_error("Matrix index out of bounds");
        }
        return data_[row][col];
    }
    
    const T& operator()(int row, int col) const {
        if (row < 0 || row >= rows_ || col < 0 || col >= cols_) {
            throw std::runtime_error("Matrix index out of bounds");
        }
        return data_[row][col];
    }
    
    int getRows() const { return rows_; }
    int getCols() const { return cols_; }
    
    void setValue(int row, int col, T value) {
        if (row < 0 || row >= rows_ || col < 0 || col >= cols_) {
            throw std::runtime_error("Matrix index out of bounds");
        }
        data_[row][col] = value;
    }
    
    T getValue(int row, int col) const {
        if (row < 0 || row >= rows_ || col < 0 || col >= cols_) {
            throw std::runtime_error("Matrix index out of bounds");
        }
        return data_[row][col];
    }
    
    void fromVector(const std::vector<T>& vec) {
        if (rows_ == 1) {
            if (vec.size() != static_cast<size_t>(cols_)) {
                throw std::runtime_error("Vector size does not match matrix columns");
            }
            for (int j = 0; j < cols_; j++) data_[0][j] = vec[j];
        } else if (cols_ == 1) {
            if (vec.size() != static_cast<size_t>(rows_)) {
                throw std::runtime_error("Vector size does not match matrix rows");
            }
            for (int i = 0; i < rows_; i++) data_[i][0] = vec[i];
        } else {
            throw std::runtime_error("Matrix is not a vector");
        }
    }
    
    std::vector<T> toVector() const {
        std::vector<T> vec;
        if (rows_ == 1) {
            for (int j = 0; j < cols_; j++) vec.push_back(data_[0][j]);
        } else if (cols_ == 1) {
            for (int i = 0; i < rows_; i++) vec.push_back(data_[i][0]);
        } else {
            throw std::runtime_error("Matrix is not a vector");
        }
        return vec;
    }
    
    void print() const {
        for (int i = 0; i < rows_; i++) {
            for (int j = 0; j < cols_; j++) {
                std::cout << std::fixed << std::setprecision(4) << data_[i][j] << " ";
            }
            std::cout << std::endl;
        }
    }

private:
    int rows_;
    int cols_;
    std::vector<std::vector<T>> data_;
    
    void allocate() {
        data_.resize(rows_);
        for (int i = 0; i < rows_; i++) {
            data_[i].resize(cols_, T(0));
        }
    }
};

#endif // MATRIX_H
