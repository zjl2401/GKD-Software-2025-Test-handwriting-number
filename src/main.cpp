#ifdef _WIN32
#define NOMINMAX
#endif
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>
#include "../include/file_reader.h"
#include "../include/socket_server.hpp"
#include <thread>
#include <mutex>
#include <atomic>
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <fstream>
class DrawingWindow {
public:
    DrawingWindow(ModelBase* model, const std::string& socketHost, int socketPort)
        : model_(model), socketHost_(socketHost), socketPort_(socketPort),
          isDrawing_(false), needUpdate_(false) {
        canvas_ = cv::Mat(400, 400, CV_8UC1, cv::Scalar(255));
        display_ = cv::Mat(400, 800, CV_8UC3, cv::Scalar(255, 255, 255));
        inferenceThread_ = std::thread(&DrawingWindow::inferenceLoop, this);
    }
    
    ~DrawingWindow() {
        running_ = false;
        if (inferenceThread_.joinable()) {
            inferenceThread_.join();
        }
    }
    
    void run() {
        std::string winTitle = "Draw a digit";
        cv::namedWindow(winTitle, cv::WINDOW_AUTOSIZE);
        cv::setMouseCallback(winTitle, mouseCallback, this);
        while (true) {
            updateDisplay();
            cv::imshow(winTitle, display_);
            
            int key = cv::waitKey(30) & 0xFF;
            if (key == 'q' || key == 27) break;
            else if (key == 'c' || key == 'C') clearCanvas();
        }
    }

private:
    ModelBase* model_;
    std::string socketHost_;
    int socketPort_;
    cv::Mat canvas_;
    cv::Mat display_;
    std::thread inferenceThread_;
    std::mutex canvasMutex_;
    std::atomic<bool> running_{true};
    std::atomic<bool> isDrawing_{false};
    std::atomic<bool> needUpdate_{false};
    
    std::vector<float> lastProbabilities_;
    int lastPrediction_;
    std::mutex resultMutex_;
    
    static void mouseCallback(int event, int x, int y, int flags, void* userdata) {
        DrawingWindow* window = static_cast<DrawingWindow*>(userdata);
        window->handleMouse(event, x, y, flags);
    }
    
    void handleMouse(int event, int x, int y, int flags) {
        if (x < 0 || x >= 400 || y < 0 || y >= 400) return;
        
        if (event == cv::EVENT_LBUTTONDOWN) {
            isDrawing_ = true;
            std::lock_guard<std::mutex> lock(canvasMutex_);
            cv::circle(canvas_, cv::Point(x, y), 10, cv::Scalar(0), -1);
            needUpdate_ = true;
        } else if (event == cv::EVENT_MOUSEMOVE && isDrawing_) {
            std::lock_guard<std::mutex> lock(canvasMutex_);
            cv::circle(canvas_, cv::Point(x, y), 10, cv::Scalar(0), -1);
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
    
    std::vector<float> preprocessImage(const cv::Mat& img) {
        cv::Mat work = img.clone();
        int x1 = work.cols, y1 = work.rows, x2 = 0, y2 = 0;
        for (int i = 0; i < work.rows; i++) {
            const uchar* row = work.ptr<uchar>(i);
            for (int j = 0; j < work.cols; j++) {
                if (row[j] < 250) {
                    if (j < x1) x1 = j;
                    if (j > x2) x2 = j;
                    if (i < y1) y1 = i;
                    if (i > y2) y2 = i;
                }
            }
        }
        cv::Mat roi;
        if (x2 >= x1 && y2 >= y1) {
            int pad = 10;
            x1 = std::max(0, x1 - pad);
            y1 = std::max(0, y1 - pad);
            x2 = std::min(work.cols - 1, x2 + pad);
            y2 = std::min(work.rows - 1, y2 + pad);
            roi = work(cv::Rect(x1, y1, x2 - x1 + 1, y2 - y1 + 1)).clone();
        } else {
            roi = work;
        }
        int rw = roi.cols, rh = roi.rows;
        if (rw <= 0) rw = 1;
        if (rh <= 0) rh = 1;
        const double targetSize = 20.0;
        double scale = std::min(targetSize / rw, targetSize / rh);
        int sw = static_cast<int>(rw * scale), sh = static_cast<int>(rh * scale);
        if (sw <= 0) sw = 1;
        if (sh <= 0) sh = 1;
        cv::Mat resized;
        cv::resize(roi, resized, cv::Size(sw, sh), 0, 0,
                  (sw < roi.cols || sh < roi.rows) ? cv::INTER_AREA : cv::INTER_LINEAR);
        cv::Mat out28(28, 28, CV_8UC1, cv::Scalar(255));
        int ox = (28 - resized.cols) / 2, oy = (28 - resized.rows) / 2;
        ox = std::max(0, std::min(ox, 28 - resized.cols));
        oy = std::max(0, std::min(oy, 28 - resized.rows));
        resized.copyTo(out28(cv::Rect(ox, oy, resized.cols, resized.rows)));
        out28.convertTo(out28, CV_32F, 1.0 / 255.0);
        std::vector<float> vec(784);
        for (int i = 0; i < 28; i++)
            for (int j = 0; j < 28; j++)
                vec[i * 28 + j] = out28.at<float>(i, j);
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
                    std::lock_guard<std::mutex> lock(resultMutex_);
                    lastProbabilities_.clear();
                    continue;
                }
                
                std::vector<float> input = preprocessImage(canvasCopy);
                
                try {
                    std::vector<float> probabilities;
                    auto t0 = std::chrono::high_resolution_clock::now();
                    if (!socketHost_.empty()) {
                        probabilities = socketForward(socketHost_, socketPort_, input);
                    } else if (model_) {
                        probabilities = model_->forward(input);
                    } else {
                        continue;
                    }
                    auto t1 = std::chrono::high_resolution_clock::now();
                    double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
                    
                    auto maxIt = std::max_element(probabilities.begin(), probabilities.end());
                    int prediction = static_cast<int>(std::distance(probabilities.begin(), maxIt));
                    
                    {
                        std::lock_guard<std::mutex> lock(resultMutex_);
                        lastProbabilities_ = probabilities;
                        lastPrediction_ = prediction;
                    }
                } catch (const std::exception& e) {
                    std::cerr << "Inference error: " << e.what() << std::endl;
                }
            }
            
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }
    
    void updateDisplay() {
        cv::Mat leftRoi = display_(cv::Rect(0, 0, 400, 400));
        {
            std::lock_guard<std::mutex> lock(canvasMutex_);
            cv::cvtColor(canvas_, leftRoi, cv::COLOR_GRAY2BGR);
        }

        cv::Mat rightPanel = display_(cv::Rect(400, 0, 400, 400));
        rightPanel.setTo(cv::Scalar(0, 0, 0));

        std::vector<float> probs;
        int pred = 0;
        {
            std::lock_guard<std::mutex> lock(resultMutex_);
            probs = lastProbabilities_;
            pred = lastPrediction_;
        }
        if (!probs.empty() && probs.size() >= 10) {
            cv::Mat leftPanel = display_(cv::Rect(0, 0, 400, 400));
            std::string predStr = "Prediction: " + std::to_string(pred);
            cv::putText(leftPanel, predStr, cv::Point(10, 32),
                        cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 255, 0), 2);

            const int nRows = 10;
            const int lineH = 36;
            const int textX = 15;
            const int barX = 95;
            const int barMaxW = 260;
            const int barH = 22;

            for (int i = 0; i < nRows; i++) {
                int y = 24 + i * lineH;
                int pct = static_cast<int>(probs[i] * 100);
                std::string rowText = std::to_string(i) + ": " + std::to_string(pct) + "%";
                cv::putText(rightPanel, rowText, cv::Point(textX, y),
                            cv::FONT_HERSHEY_SIMPLEX, 0.65, cv::Scalar(0, 0, 255), 1);
                int w = static_cast<int>(probs[i] * barMaxW);
                if (w > 0) {
                    if (w < 2) w = 2;
                    cv::rectangle(rightPanel,
                                 cv::Point(barX, y - barH + 4),
                                 cv::Point(barX + w, y + 4),
                                 cv::Scalar(0, 255, 0), -1);
                }
            }
        } else {
            cv::putText(rightPanel, "Draw a digit on the left", cv::Point(30, 200),
                        cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(255, 255, 255), 1);
        }
        cv::putText(rightPanel, "c: clear  q/ESC: quit", cv::Point(30, 378),
                    cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(180, 180, 180), 1);
    }
};

int main(int argc, char** argv) {
    try {
        bool usePlus = false;
        bool serverMode = false;
        std::string socketHost;
        int socketPort = 12345;
        for (int i = 1; i < argc; i++) {
            std::string arg = argv[i];
            if (arg == "plus") usePlus = true;
            else if (arg == "--server") serverMode = true;
            else if (arg == "--socket" && i + 1 < argc) socketHost = argv[++i];
        }
        
        std::string modelFolder = usePlus ? "mnist-fc-plus" : "mnist-fc";
        std::vector<std::string> possiblePaths = { "../" + modelFolder, modelFolder, "../../" + modelFolder };
        std::string modelPath;
        bool found = false;
        for (const auto& path : possiblePaths) {
            std::ifstream f(path + "/fc1.weight", std::ios::binary);
            if (f.good()) { modelPath = path; found = true; break; }
        }
        std::unique_ptr<ModelBase> model;
        if (serverMode || socketHost.empty()) {
            if (!found) {
                std::cerr << "Cannot find model folder: " << modelFolder << std::endl;
                std::cerr << "Searched: " << possiblePaths[0] << ", " << possiblePaths[1] << ", " << possiblePaths[2] << std::endl;
                std::cerr << "Tip: Copy " << modelFolder << " to the same directory as the .exe (e.g. build/ or build/Release)" << std::endl;
                throw std::runtime_error("Cannot find " + modelFolder + " directory.");
            }
            model = createModel(modelPath);
        }
        if (serverMode) {
            SocketServer server(model.get(), socketPort);
            server.run();
            return 0;
        }
        
        DrawingWindow window(model.get(), socketHost, socketPort);
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
