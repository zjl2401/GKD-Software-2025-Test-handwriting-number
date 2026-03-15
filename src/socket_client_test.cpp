#include "../include/socket_server.hpp"
#include <iostream>
#include <iomanip>
#include <vector>
#include <random>

int main() {
    std::string host = "127.0.0.1";
    int port = 12345;
    
    auto test = [&](const std::string& name, const std::vector<float>& input) {
        std::cout << "\n" << name << " (" << input.size() << " elements):" << std::endl;
        try {
            auto result = socketForward(host, port, input);
            std::cout << "  Response (" << result.size() << " probs): ";
            int pred = 0;
            for (size_t i = 0; i < result.size(); i++) {
                std::cout << std::fixed << std::setprecision(3) << result[i] << " ";
                if (result[i] > result[pred]) pred = static_cast<int>(i);
            }
            std::cout << "\n  Prediction: " << pred << std::endl;
            return true;
        } catch (const std::exception& e) {
            std::cerr << "  Error: " << e.what() << std::endl;
            return false;
        }
    };
    
    std::cout << "Connecting to " << host << ":" << port << std::endl;
    std::cout << "Make sure server is running: HandwritingNumberRecognition --server" << std::endl;
    
    std::vector<float> allBlack(784, 0.0f);
    std::vector<float> allWhite(784, 1.0f);
    std::vector<float> randomVec(784);
    std::mt19937 rng(42);
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    for (float& v : randomVec) v = dist(rng);
    
    bool ok = test("All black (0)", allBlack);
    ok = test("All white (1)", allWhite) && ok;
    ok = test("Random", randomVec) && ok;
    
    if (ok) {
        std::cout << "\nAll tests passed!" << std::endl;
        return 0;
    } else {
        std::cout << "\nSome tests failed." << std::endl;
        return 1;
    }
}
