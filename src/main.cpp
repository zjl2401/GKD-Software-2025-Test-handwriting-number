#include "loader.hpp"
#include "model.hpp"
#include "socket_server.hpp"
#include "socket_client.hpp"
#include <opencv2/opencv.hpp>
#include <iostream>
#include <chrono>
#include <atomic>
#include <thread>
#include <mutex>

cv::Mat canvas;
std::atomic<bool> needPredict{false};
std::atomic<bool> running{true};
std::vector<float> lastProbs(10, 0);
std::mutex probsMutex;
std::string useSocket;
std::string modelDir;

void mouseCallback(int event, int x, int y, int, void*) {
    static bool drawing = false;
    if (event == cv::EVENT_LBUTTONDOWN) drawing = true;
    if (event == cv::EVENT_LBUTTONUP) drawing = false;
    if (drawing && x >= 0 && x < 280 && y >= 0 && y < 280) {
        for (int dy = -5; dy <= 5; ++dy)
            for (int dx = -5; dx <= 5; ++dx) {
                int nx = x + dx, ny = y + dy;
                if (nx >= 0 && nx < 280 && ny >= 0 && ny < 280)
                    canvas.at<uchar>(ny, nx) = 0;
            }
        needPredict = true;
    }
}

std::vector<float> preprocess(const cv::Mat& img) {
    cv::Mat gray, small;
    if (img.channels() > 1) cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);
    else gray = img.clone();
    cv::resize(gray, small, cv::Size(28, 28));
    std::vector<float> vec(784);
    for (int i = 0; i < 28; ++i)
        for (int j = 0; j < 28; ++j)
            vec[i * 28 + j] = (255.0f - small.at<uchar>(i, j)) / 255.0f;
    return vec;
}

void runGUI(std::unique_ptr<ModelBase> model) {
    canvas = cv::Mat(280, 280, CV_8UC1, 255);
    cv::namedWindow("MNIST", cv::WINDOW_NORMAL);
    cv::setMouseCallback("MNIST", mouseCallback);
    
    std::thread predictThread([&model]() {
        while (running) {
            if (!needPredict) { std::this_thread::sleep_for(std::chrono::milliseconds(50)); continue; }
            needPredict = false;
            cv::Mat drawArea = canvas(cv::Rect(0, 0, 280, 280));
            auto vec = preprocess(drawArea);
            std::vector<float> probs;
            if (!useSocket.empty()) {
                probs = socketForward(useSocket, 12345, vec);
            } else if (model) {
                probs = model->forward(vec);
            }
            if (probs.size() == 10) {
                std::lock_guard<std::mutex> lock(probsMutex);
                for (int i = 0; i < 10; ++i) lastProbs[i] = probs[i];
            }
        }
    });
    
    while (running) {
        cv::Mat display(400, 500, CV_8UC3, cv::Scalar(255, 255, 255));
        cv::Mat roi = display(cv::Rect(10, 10, 280, 280));
        cv::cvtColor(canvas, roi, cv::COLOR_GRAY2BGR);
        
        std::vector<float> probsCopy(10);
        { std::lock_guard<std::mutex> lock(probsMutex); probsCopy = lastProbs; }
        int maxIdx = 0;
        for (int i = 1; i < 10; ++i) if (probsCopy[i] > probsCopy[maxIdx]) maxIdx = i;
        cv::putText(display, std::string("Pred: ") + char('0' + maxIdx),
                    cv::Point(310, 50), cv::FONT_HERSHEY_SIMPLEX, 1.5, cv::Scalar(0, 0, 0), 2);
        
        int barW = 15, gap = 5;
        for (int i = 0; i < 10; ++i) {
            int h = (int)(probsCopy[i] * 200);
            cv::rectangle(display, cv::Point(310 + i * (barW + gap), 300 - h),
                         cv::Point(310 + i * (barW + gap) + barW, 300),
                         i == maxIdx ? cv::Scalar(0, 255, 0) : cv::Scalar(200, 200, 200), -1);
            cv::putText(display, std::to_string(i), cv::Point(308 + i * (barW + gap), 320),
                        cv::FONT_HERSHEY_SIMPLEX, 0.4, cv::Scalar(0, 0, 0));
        }
        
        cv::imshow("MNIST", display);
        int k = cv::waitKey(30);
        if (k == 27) break;
        if (k == 'c' || k == 'C') canvas = cv::Mat(280, 280, CV_8UC1, 255);
    }
    running = false;
    predictThread.join();
}

int main(int argc, char** argv) {
    bool isPlus = false, isServer = false;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "plus") isPlus = true;
        else if (arg == "--server") isServer = true;
        else if (arg == "--socket" && i + 1 < argc) useSocket = argv[++i];
    }
    
    modelDir = isPlus ? "mnist-fc-plus" : "mnist-fc";
    
    std::unique_ptr<ModelBase> model;
    if (!useSocket.empty()) {
        std::cout << "Using socket: " << useSocket << std::endl;
    } else {
        try {
            model = createModel(modelDir);
        } catch (const std::exception& e) {
            std::cerr << "Load model failed: " << e.what() << std::endl;
            return 1;
        }
    }
    
    if (isServer) {
        if (!model) { std::cerr << "Server needs local model" << std::endl; return 1; }
        std::cout << "Socket server on port 12345" << std::endl;
        SocketServer srv(std::move(model), 12345);
        srv.run();
        return 0;
    }
    
    runGUI(std::move(model));
    return 0;
}
