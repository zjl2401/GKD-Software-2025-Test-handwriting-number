#ifndef MATRIX_H
#define MATRIX_H

#include <vector>
#include <iostream>
#include <stdexcept>
#include <iomanip>

class Matrix {
public:
    // 构造函数
    Matrix() : rows_(0), cols_(0) {
    }
    
    Matrix(int rows, int cols) : rows_(rows), cols_(cols) {
        allocate();
    }
    
    Matrix(int rows, int cols, float value) : rows_(rows), cols_(cols) {
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
    
    // 析构函数
    ~Matrix() {
    }
    
    // 赋值运算符
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
    
    // 矩阵运算
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
        
        Matrix result(rows_, other.cols_, 0.0f);
        // O(n^3) 矩阵乘法
        for (int i = 0; i < rows_; i++) {
            for (int j = 0; j < other.cols_; j++) {
                for (int k = 0; k < cols_; k++) {
                    result.data_[i][j] += data_[i][k] * other.data_[k][j];
                }
            }
        }
        return result;
    }
    
    // 元素访问
    float& operator()(int row, int col) {
        if (row < 0 || row >= rows_ || col < 0 || col >= cols_) {
            throw std::runtime_error("Matrix index out of bounds");
        }
        return data_[row][col];
    }
    
    const float& operator()(int row, int col) const {
        if (row < 0 || row >= rows_ || col < 0 || col >= cols_) {
            throw std::runtime_error("Matrix index out of bounds");
        }
        return data_[row][col];
    }
    
    // 获取维度
    int getRows() const { return rows_; }
    int getCols() const { return cols_; }
    
    // 设置值
    void setValue(int row, int col, float value) {
        if (row < 0 || row >= rows_ || col < 0 || col >= cols_) {
            throw std::runtime_error("Matrix index out of bounds");
        }
        data_[row][col] = value;
    }
    
    float getValue(int row, int col) const {
        if (row < 0 || row >= rows_ || col < 0 || col >= cols_) {
            throw std::runtime_error("Matrix index out of bounds");
        }
        return data_[row][col];
    }
    
    // 从向量初始化（用于1D向量）
    void fromVector(const std::vector<float>& vec) {
        if (rows_ == 1) {
            // 行向量
            if (vec.size() != static_cast<size_t>(cols_)) {
                throw std::runtime_error("Vector size does not match matrix columns");
            }
            for (int j = 0; j < cols_; j++) {
                data_[0][j] = vec[j];
            }
        } else if (cols_ == 1) {
            // 列向量
            if (vec.size() != static_cast<size_t>(rows_)) {
                throw std::runtime_error("Vector size does not match matrix rows");
            }
            for (int i = 0; i < rows_; i++) {
                data_[i][0] = vec[i];
            }
        } else {
            throw std::runtime_error("Matrix is not a vector");
        }
    }
    
    std::vector<float> toVector() const {
        std::vector<float> vec;
        if (rows_ == 1) {
            // 行向量
            for (int j = 0; j < cols_; j++) {
                vec.push_back(data_[0][j]);
            }
        } else if (cols_ == 1) {
            // 列向量
            for (int i = 0; i < rows_; i++) {
                vec.push_back(data_[i][0]);
            }
        } else {
            throw std::runtime_error("Matrix is not a vector");
        }
        return vec;
    }
    
    // 打印矩阵（用于调试）
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
    std::vector<std::vector<float>> data_;
    
    void allocate() {
        data_.resize(rows_);
        for (int i = 0; i < rows_; i++) {
            data_[i].resize(cols_, 0.0f);
        }
    }
};

#endif // MATRIX_H
