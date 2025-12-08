#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>
#include "../include/file_reader.h"
#include <thread>
#include <mutex>
#include <atomic>
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <fstream>
#include <iomanip>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <limits.h>
#endif

class DrawingWindow {
public:
    DrawingWindow(Model& model) : model_(model), isDrawing_(false), needUpdate_(false) {
        // 创建主画布（用于绘制）
        canvas_ = cv::Mat(400, 400, CV_8UC1, cv::Scalar(255));
        
        // 创建显示窗口（包含画布和结果）
        display_ = cv::Mat(400, 800, CV_8UC1, cv::Scalar(255));
        
        // 启动推理线程
        inferenceThread_ = std::thread(&DrawingWindow::inferenceLoop, this);
    }
    
    ~DrawingWindow() {
        running_ = false;
        if (inferenceThread_.joinable()) {
            inferenceThread_.join();
        }
    }
    
    void run() {
        std::cout << "Creating OpenCV window..." << std::endl;
        cv::namedWindow("Handwriting Number Recognition", cv::WINDOW_AUTOSIZE);
        cv::setMouseCallback("Handwriting Number Recognition", mouseCallback, this);
        std::cout << "Window created successfully!" << std::endl;
        std::cout << "Window should be visible now. If not, check your taskbar or press Alt+Tab." << std::endl;
        
        while (true) {
            updateDisplay();
            cv::imshow("Handwriting Number Recognition", display_);
            
            int key = cv::waitKey(30) & 0xFF;
            if (key == 'q' || key == 27) {  // 'q' 或 ESC 退出
                break;
            } else if (key == 'c' || key == 'C') {  // 'c' 清空画布
                clearCanvas();
            }
        }
    }

private:
    Model& model_;
    cv::Mat canvas_;
    cv::Mat display_;
    std::thread inferenceThread_;
    std::mutex canvasMutex_;
    std::atomic<bool> running_{true};
    std::atomic<bool> isDrawing_{false};
    std::atomic<bool> needUpdate_{false};
    
    std::vector<float> lastProbabilities_;
    int lastPrediction_;
    
    static void mouseCallback(int event, int x, int y, int flags, void* userdata) {
        DrawingWindow* window = static_cast<DrawingWindow*>(userdata);
        window->handleMouse(event, x, y, flags);
    }
    
    void handleMouse(int event, int x, int y, int flags) {
        // 只处理主画布区域（左侧400x400）
        if (x < 0 || x >= 400 || y < 0 || y >= 400) {
            return;
        }
        
        if (event == cv::EVENT_LBUTTONDOWN) {
            isDrawing_ = true;
            std::lock_guard<std::mutex> lock(canvasMutex_);
            cv::circle(canvas_, cv::Point(x, y), 8, cv::Scalar(0), -1);
            needUpdate_ = true;
        } else if (event == cv::EVENT_MOUSEMOVE && isDrawing_) {
            std::lock_guard<std::mutex> lock(canvasMutex_);
            cv::circle(canvas_, cv::Point(x, y), 8, cv::Scalar(0), -1);
            needUpdate_ = true;
        } else if (event == cv::EVENT_LBUTTONUP) {
            isDrawing_ = false;
        }
    }
    
    void clearCanvas() {
        std::lock_guard<std::mutex> lock(canvasMutex_);
        canvas_ = cv::Mat(400, 400, CV_8UC1, cv::Scalar(255));
        needUpdate_ = true;
    }
    
    Matrix preprocessImage(const cv::Mat& img) {
        // 缩小到28x28
        cv::Mat resized;
        cv::resize(img, resized, cv::Size(28, 28));
        
        // 转换为浮点数并归一化到0-1
        resized.convertTo(resized, CV_32F, 1.0 / 255.0);
        
        // 倒置：MNIST期望黑色背景(0)和白色笔画(1)
        // 我们有：白色背景(1)和黑色笔画(0)
        // 所以需要倒置
        cv::Mat inverted = 1.0f - resized;
        
        // 保存预处理后的图像到PNG文件（用于调试）
        cv::Mat preview;
        inverted.convertTo(preview, CV_8UC1, 255.0);  // 转换回0-255范围
        cv::imwrite("preprocessed_input.png", preview);
        
        // 拍扁成一维向量
        Matrix input(1, 784);
        
        for (int i = 0; i < 28; i++) {
            for (int j = 0; j < 28; j++) {
                float value = inverted.at<float>(i, j);
                input(0, i * 28 + j) = value;
            }
        }
        
        return input;
    }
    
    void inferenceLoop() {
        while (running_) {
            if (needUpdate_) {
                cv::Mat canvasCopy;
                {
                    std::lock_guard<std::mutex> lock(canvasMutex_);
                    canvasCopy = canvas_.clone();
                    needUpdate_ = false;
                }
                
                // 检查画布是否为空（全白）
                cv::Scalar meanVal = cv::mean(canvasCopy);
                if (meanVal[0] > 250.0) {  // 如果平均像素值接近255，说明画布基本是空的
                    // 清空概率，不进行推理
                    lastProbabilities_.clear();
                    continue;
                }
                
                // 预处理
                Matrix input = preprocessImage(canvasCopy);
                
                // 推理
                try {
                    std::vector<float> probabilities = model_.forward(input);
                    
                    // 找到概率最大的数字
                    auto maxIt = std::max_element(probabilities.begin(), probabilities.end());
                    int prediction = static_cast<int>(std::distance(probabilities.begin(), maxIt));
                    
                    // 调试输出：打印所有概率值（仅第一次或当预测总是1时）
                    static int inferenceCount = 0;
                    inferenceCount++;
                    if (inferenceCount <= 3 || prediction == 1) {
                        std::cout << "Inference #" << inferenceCount << " - Probabilities: ";
                        for (size_t i = 0; i < probabilities.size(); i++) {
                            std::cout << i << ":" << std::fixed << std::setprecision(3) 
                                     << probabilities[i] << " ";
                        }
                        std::cout << "-> Prediction: " << prediction << std::endl;
                    }
                    
                    lastProbabilities_ = probabilities;
                    lastPrediction_ = prediction;
                } catch (const std::exception& e) {
                    std::cerr << "Inference error: " << e.what() << std::endl;
                }
            }
            
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }
    
    void updateDisplay() {
        // 复制画布到显示窗口左侧
        {
            std::lock_guard<std::mutex> lock(canvasMutex_);
            canvas_.copyTo(display_(cv::Rect(0, 0, 400, 400)));
        }
        
        // 在右侧绘制结果
        cv::Mat rightPanel = display_(cv::Rect(400, 0, 400, 400));
        rightPanel = cv::Scalar(255);
        
        // 绘制预测结果
        if (!lastProbabilities_.empty()) {
            // 显示预测数字（大字体）
            std::string predText = "Prediction: " + std::to_string(lastPrediction_);
            cv::putText(rightPanel, predText, cv::Point(50, 50), 
                       cv::FONT_HERSHEY_SIMPLEX, 1.5, cv::Scalar(0), 3);
            
            // 绘制10个柱状图
            int barWidth = 30;
            int barSpacing = 10;
            int startX = 50;
            int startY = 350;
            int maxHeight = 250;
            
            for (int i = 0; i < 10; i++) {
                int x = startX + i * (barWidth + barSpacing);
                int height = static_cast<int>(lastProbabilities_[i] * maxHeight);
                int y = startY - height;
                
                // 绘制柱状图
                cv::rectangle(rightPanel, 
                            cv::Point(x, y), 
                            cv::Point(x + barWidth, startY),
                            cv::Scalar(0), -1);
                
                // 绘制数字标签
                cv::putText(rightPanel, std::to_string(i), 
                          cv::Point(x + 8, startY + 20),
                          cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0), 2);
                
                // 绘制概率值
                std::string probText = std::to_string(static_cast<int>(lastProbabilities_[i] * 100)) + "%";
                cv::putText(rightPanel, probText,
                          cv::Point(x, y - 5),
                          cv::FONT_HERSHEY_SIMPLEX, 0.4, cv::Scalar(0), 1);
            }
        } else {
            cv::putText(rightPanel, "Draw a number", cv::Point(50, 200),
                       cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0), 2);
        }
        
        // 绘制说明文字
        cv::putText(rightPanel, "Press 'c' to clear", cv::Point(50, 380),
                   cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0), 1);
    }
};

int main(int argc, char** argv) {
    try {
        // 获取可执行文件所在目录
        std::string exePath;
        #ifdef _WIN32
            char buffer[MAX_PATH];
            GetModuleFileNameA(NULL, buffer, MAX_PATH);
            exePath = buffer;
            size_t pos = exePath.find_last_of("\\/");
            if (pos != std::string::npos) {
                exePath = exePath.substr(0, pos);
            }
        #else
            char buffer[1024];
            ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
            if (len != -1) {
                buffer[len] = '\0';
                exePath = buffer;
                size_t pos = exePath.find_last_of("/");
                if (pos != std::string::npos) {
                    exePath = exePath.substr(0, pos);
                }
            }
        #endif
        
        // 确定模型路径：尝试多个可能的路径
        std::vector<std::string> possiblePaths = {
            "../mnist-fc",  // 从 build 目录向上查找
            "mnist-fc",     // 当前目录
            "../../mnist-fc" // 备用路径
        };
        
        std::string modelPath;
        bool found = false;
        std::cout << "Searching for model files..." << std::endl;
        for (const auto& path : possiblePaths) {
            std::string testPath = path + "/fc1.weight";
            std::ifstream testFile(testPath, std::ios::binary);
            if (testFile.good()) {
                modelPath = path;
                found = true;
                testFile.close();
                std::cout << "Found model at: " << modelPath << std::endl;
                break;
            } else {
                std::cout << "  Not found: " << testPath << std::endl;
            }
        }
        
        if (!found) {
            std::cerr << "Error: Cannot find mnist-fc directory." << std::endl;
            std::cerr << "Searched paths:" << std::endl;
            for (const auto& path : possiblePaths) {
                std::cerr << "  - " << path << "/fc1.weight" << std::endl;
            }
            throw std::runtime_error("Cannot find mnist-fc directory. Please ensure it exists in the project root.");
        }
        
        // 加载模型
        std::cout << "Loading model from: " << modelPath << std::endl;
        Model model = ModelLoader::loadModel(modelPath);
        std::cout << "Model loaded successfully!" << std::endl;
        
        // 创建并运行绘制窗口
        DrawingWindow window(model);
        window.run();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        #ifdef _WIN32
            std::cout << "Press any key to exit..." << std::endl;
            std::cin.get();
        #endif
        return 1;
    }
    
    return 0;
}

