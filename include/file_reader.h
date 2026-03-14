#ifndef FILE_READER_H
#define FILE_READER_H

#include "matrix.h"
#include "model.h"
#include <string>
#include <fstream>

// 从二进制文件读取矩阵
inline Matrix readMatrixFromBinary(const std::string& filename, int rows, int cols) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file: " + filename);
    }
    
    Matrix matrix(rows, cols);
    
    // 读取二进制数据：每4个字节构成一个float
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            float value;
            file.read(reinterpret_cast<char*>(&value), sizeof(float));
            if (!file) {
                throw std::runtime_error("Error reading from file: " + filename);
            }
            matrix(i, j) = value;
        }
    }
    
    file.close();
    return matrix;
}

// 从二进制文件读取模型参数（不使用JSON）
class ModelLoader {
public:
    static Model loadModel(const std::string& folderPath) {
        // 模型维度（从meta.json中已知）
        // fc1.weight 文件大小：1568000 bytes = 500*784*4 bytes
        //  所以维度是 [500, 784] 或者存储时是行优先
        // fc1.bias: [1, 500]
        // fc2.weight: [10, 500]
        // fc2.bias: [1, 10]
        
        std::string basePath = folderPath;
        if (basePath.back() != '/' && basePath.back() != '\\') {
            basePath += "/";
        }
        
        // 读取fc1层的参数 - 不转置，直接读取
        // PyTorch的Linear层权重存储为 [out_features, in_features]
        // 文件中应该是 [500, 784]，行优先存储
        Matrix weight1 = readMatrixFromBinary(basePath + "fc1.weight", 500, 784);
        Matrix bias1 = readMatrixFromBinary(basePath + "fc1.bias", 1, 500);
        
        // 读取fc2层的参数
        Matrix weight2 = readMatrixFromBinary(basePath + "fc2.weight", 10, 500);
        Matrix bias2 = readMatrixFromBinary(basePath + "fc2.bias", 1, 10);
        
        // 创建并返回Model
        return Model(weight1, bias1, weight2, bias2);
    }
};

#endif // FILE_READER_H
