#pragma once

#include "matrix.hpp"
#include "model.hpp"
#include <fstream>
#include <string>
#include <nlohmann/json.hpp>

template<typename T>
Matrix<T> loadMatrix(const std::string& filepath, size_t rows, size_t cols) {
    std::ifstream f(filepath, std::ios::binary);
    if (!f) throw std::runtime_error("Cannot open file: " + filepath);
    
    size_t size = rows * cols;
    std::vector<T> data(size);
    f.read(reinterpret_cast<char*>(data.data()), size * sizeof(T));
    return Matrix<T>(rows, cols, data);
}

inline void loadModelFromDir(const std::string& dir, nlohmann::json& meta,
    Matrix<float>& w1, Matrix<float>& b1, Matrix<float>& w2, Matrix<float>& b2) {
    std::string prefix = dir.back() == '/' || dir.back() == '\\' ? dir : dir + "/";
    std::ifstream metaFile(prefix + "meta.json");
    metaFile >> meta;
    auto fc1w = meta["fc1.weight"], fc1b = meta["fc1.bias"];
    auto fc2w = meta["fc2.weight"], fc2b = meta["fc2.bias"];
    w1 = loadMatrix<float>(prefix + "fc1.weight", fc1w[0], fc1w[1]);
    b1 = loadMatrix<float>(prefix + "fc1.bias", fc1b[0], fc1b[1]);
    w2 = loadMatrix<float>(prefix + "fc2.weight", fc2w[0], fc2w[1]);
    b2 = loadMatrix<float>(prefix + "fc2.bias", fc2b[0], fc2b[1]);
}

inline void loadModelFromDir(const std::string& dir, nlohmann::json& meta,
    Matrix<double>& w1, Matrix<double>& b1, Matrix<double>& w2, Matrix<double>& b2) {
    std::string prefix = dir.back() == '/' || dir.back() == '\\' ? dir : dir + "/";
    std::ifstream metaFile(prefix + "meta.json");
    metaFile >> meta;
    auto fc1w = meta["fc1.weight"], fc1b = meta["fc1.bias"];
    auto fc2w = meta["fc2.weight"], fc2b = meta["fc2.bias"];
    w1 = loadMatrix<double>(prefix + "fc1.weight", fc1w[0], fc1w[1]);
    b1 = loadMatrix<double>(prefix + "fc1.bias", fc1b[0], fc1b[1]);
    w2 = loadMatrix<double>(prefix + "fc2.weight", fc2w[0], fc2w[1]);
    b2 = loadMatrix<double>(prefix + "fc2.bias", fc2b[0], fc2b[1]);
}

inline std::unique_ptr<ModelBase> createModel(const std::string& dir) {
    nlohmann::json meta;
    std::string path = (dir.empty() || dir.back() == '/' || dir.back() == '\\') ? dir : dir + "/";
    
    std::ifstream metaFile(path + "meta.json");
    metaFile >> meta;
    
    std::string type = meta.value("type", "fp32");
    if (type == "fp64") {
        Matrix<double> w1, b1, w2, b2;
        loadModelFromDir(path.empty() ? dir : path, meta, w1, b1, w2, b2);
        return std::make_unique<Model<double>>(w1, b1, w2, b2);
    } else {
        Matrix<float> w1, b1, w2, b2;
        loadModelFromDir(path.empty() ? dir : path, meta, w1, b1, w2, b2);
        return std::make_unique<Model<float>>(w1, b1, w2, b2);
    }
}
