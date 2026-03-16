#ifndef FILE_READER_H
#define FILE_READER_H

#include "matrix.h"
#include "model.h"
#include <nlohmann/json.hpp>
#include <string>
#include <fstream>
#include <memory>
#include <vector>

template<typename T>
inline Matrix<T> readMatrixFromBinary(const std::string& filename, int rows, int cols) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file: " + filename);
    }
    
    Matrix<T> matrix(rows, cols);
    
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            T value;
            file.read(reinterpret_cast<char*>(&value), sizeof(T));
            if (!file) {
                throw std::runtime_error("Error reading from file: " + filename);
            }
            matrix(i, j) = value;
        }
    }
    
    file.close();
    return matrix;
}

// 在常见相对路径下查找模型目录（存在 fc1.weight 的目录），找不到返回空字符串
inline std::string findModelPath(const std::string& modelFolder) {
    std::vector<std::string> possiblePaths = { "../" + modelFolder, modelFolder, "../../" + modelFolder };
    for (const auto& path : possiblePaths) {
        std::ifstream f(path + "/fc1.weight", std::ios::binary);
        if (f.good()) return path;
    }
    return "";
}

// 根据 meta.json 的 type 加载 Model<float> 或 Model<double>，返回 ModelBase 指针
inline std::unique_ptr<ModelBase> createModel(const std::string& folderPath) {
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
    
    std::string type = meta.value("type", "fp32");
    
    auto fc1w = meta["fc1.weight"];
    auto fc1b = meta["fc1.bias"];
    auto fc2w = meta["fc2.weight"];
    auto fc2b = meta["fc2.bias"];
    
    int w1_rows = fc1w[0].get<int>();
    int w1_cols = fc1w[1].get<int>();
    int b1_rows = fc1b[0].get<int>();
    int b1_cols = fc1b[1].get<int>();
    int w2_rows = fc2w[0].get<int>();
    int w2_cols = fc2w[1].get<int>();
    int b2_rows = fc2b[0].get<int>();
    int b2_cols = fc2b[1].get<int>();
    
    if (type == "fp64") {
        Matrix<double> weight1 = readMatrixFromBinary<double>(basePath + "fc1.weight", w1_rows, w1_cols);
        Matrix<double> bias1 = readMatrixFromBinary<double>(basePath + "fc1.bias", b1_rows, b1_cols);
        Matrix<double> weight2 = readMatrixFromBinary<double>(basePath + "fc2.weight", w2_rows, w2_cols);
        Matrix<double> bias2 = readMatrixFromBinary<double>(basePath + "fc2.bias", b2_rows, b2_cols);
        return std::make_unique<Model<double>>(weight1, bias1, weight2, bias2);
    } else {
        Matrix<float> weight1 = readMatrixFromBinary<float>(basePath + "fc1.weight", w1_rows, w1_cols);
        Matrix<float> bias1 = readMatrixFromBinary<float>(basePath + "fc1.bias", b1_rows, b1_cols);
        Matrix<float> weight2 = readMatrixFromBinary<float>(basePath + "fc2.weight", w2_rows, w2_cols);
        Matrix<float> bias2 = readMatrixFromBinary<float>(basePath + "fc2.bias", b2_rows, b2_cols);
        return std::make_unique<Model<float>>(weight1, bias1, weight2, bias2);
    }
}

#endif // FILE_READER_H
