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
    DrawingWindow(ModelBase& model) : model_(model), isDrawing_(false), needUpdate_(false) {
        canvas_ = cv::Mat(400, 400, CV_8UC1, cv::Scalar(255));
        display_ = cv::Mat(400, 800, CV_8UC1, cv::Scalar(255));
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
            if (key == 'q' || key == 27) break;
            else if (key == 'c' || key == 'C') clearCanvas();
        }
    }

private:
    ModelBase& model_;
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
        if (x < 0 || x >= 400 || y < 0 || y >= 400) return;
        
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
    
    // 预处理：返回 784 维 float 向量（0~1 归一化，白底黑字）
    std::vector<float> preprocessImage(const cv::Mat& img) {
        cv::Mat resized;
        cv::resize(img, resized, cv::Size(28, 28));
        resized.convertTo(resized, CV_32F, 1.0 / 255.0);
        cv::Mat inverted = 1.0f - resized;
        
        std::vector<float> vec(784);
        for (int i = 0; i < 28; i++) {
            for (int j = 0; j < 28; j++) {
                vec[i * 28 + j] = inverted.at<float>(i, j);
            }
        }
        return vec;
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
                
                cv::Scalar meanVal = cv::mean(canvasCopy);
                if (meanVal[0] > 250.0) {
                    lastProbabilities_.clear();
                    continue;
                }
                
                std::vector<float> input = preprocessImage(canvasCopy);
                
                try {
                    std::vector<float> probabilities = model_.forward(input);
                    auto maxIt = std::max_element(probabilities.begin(), probabilities.end());
                    int prediction = static_cast<int>(std::distance(probabilities.begin(), maxIt));
                    
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
        {
            std::lock_guard<std::mutex> lock(canvasMutex_);
            canvas_.copyTo(display_(cv::Rect(0, 0, 400, 400)));
        }
        
        cv::Mat rightPanel = display_(cv::Rect(400, 0, 400, 400));
        rightPanel = cv::Scalar(255);
        
        if (!lastProbabilities_.empty()) {
            std::string predText = "Prediction: " + std::to_string(lastPrediction_);
            cv::putText(rightPanel, predText, cv::Point(50, 50),
                       cv::FONT_HERSHEY_SIMPLEX, 1.5, cv::Scalar(0), 3);
            
            int barWidth = 30;
            int barSpacing = 10;
            int startX = 50;
            int startY = 350;
            int maxHeight = 250;
            
            for (int i = 0; i < 10; i++) {
                int x = startX + i * (barWidth + barSpacing);
                int height = static_cast<int>(lastProbabilities_[i] * maxHeight);
                int y = startY - height;
                
                cv::rectangle(rightPanel,
                            cv::Point(x, y),
                            cv::Point(x + barWidth, startY),
                            cv::Scalar(0), -1);
                cv::putText(rightPanel, std::to_string(i),
                          cv::Point(x + 8, startY + 20),
                          cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0), 2);
                std::string probText = std::to_string(static_cast<int>(lastProbabilities_[i] * 100)) + "%";
                cv::putText(rightPanel, probText,
                          cv::Point(x, y - 5),
                          cv::FONT_HERSHEY_SIMPLEX, 0.4, cv::Scalar(0), 1);
            }
        } else {
            cv::putText(rightPanel, "Draw a number", cv::Point(50, 200),
                       cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0), 2);
        }
        
        cv::putText(rightPanel, "Press 'c' to clear", cv::Point(50, 380),
                   cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0), 1);
    }
};

int main(int argc, char** argv) {
    try {
        // 解析参数：plus 表示使用 mnist-fc-plus
        bool usePlus = false;
        for (int i = 1; i < argc; i++) {
            if (std::string(argv[i]) == "plus") {
                usePlus = true;
                break;
            }
        }
        
        std::string modelFolder = usePlus ? "mnist-fc-plus" : "mnist-fc";
        std::cout << "Using model: " << modelFolder << std::endl;
        
        // 确定模型路径
        std::vector<std::string> possiblePaths = {
            "../" + modelFolder,
            modelFolder,
            "../../" + modelFolder
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
            std::cerr << "Error: Cannot find " << modelFolder << " directory." << std::endl;
            throw std::runtime_error("Cannot find model directory. Run with 'plus' for mnist-fc-plus.");
        }
        
        std::cout << "Loading model from: " << modelPath << std::endl;
        std::unique_ptr<ModelBase> model = createModel(modelPath);
        std::cout << "Model loaded successfully!" << std::endl;
        
        DrawingWindow window(*model);
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
