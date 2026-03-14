#ifndef FILE_READER_H
#define FILE_READER_H

#include "matrix.h"
#include "model.h"
#include <nlohmann/json.hpp>
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

// 从 meta.json 解析模型维度并加载模型参数
class ModelLoader {
public:
    static Model loadModel(const std::string& folderPath) {
        std::string basePath = folderPath;
        if (basePath.back() != '/' && basePath.back() != '\\') {
            basePath += "/";
        }
        
        std::string metaPath = basePath + "meta.json";
        std::ifstream metaFile(metaPath);
        if (!metaFile.is_open()) {
            throw std::runtime_error("Cannot open meta.json: " + metaPath);
        }
        
        nlohmann::json meta;
        metaFile >> meta;
        metaFile.close();
        
        // 从 meta.json 读取各矩阵维度
        // meta 中格式为 [dim0, dim1]，权重为 [in_features, out_features]
        // PyTorch Linear 存储为 [out_features, in_features]，故读取时 rows=dim1, cols=dim0
        auto fc1w = meta["fc1.weight"];
        auto fc1b = meta["fc1.bias"];
        auto fc2w = meta["fc2.weight"];
        auto fc2b = meta["fc2.bias"];
        
        int w1_rows = fc1w[1].get<int>();
        int w1_cols = fc1w[0].get<int>();
        int b1_rows = fc1b[0].get<int>();
        int b1_cols = fc1b[1].get<int>();
        int w2_rows = fc2w[1].get<int>();
        int w2_cols = fc2w[0].get<int>();
        int b2_rows = fc2b[0].get<int>();
        int b2_cols = fc2b[1].get<int>();
        
        // 读取四个矩阵
        Matrix weight1 = readMatrixFromBinary(basePath + "fc1.weight", w1_rows, w1_cols);
        Matrix bias1 = readMatrixFromBinary(basePath + "fc1.bias", b1_rows, b1_cols);
        Matrix weight2 = readMatrixFromBinary(basePath + "fc2.weight", w2_rows, w2_cols);
        Matrix bias2 = readMatrixFromBinary(basePath + "fc2.bias", b2_rows, b2_cols);
        
        return Model(weight1, bias1, weight2, bias2);
    }
};

#endif // FILE_READER_H
