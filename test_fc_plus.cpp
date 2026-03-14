#include "include/matrix.h"
#include "include/file_reader.h"
#include "include/model.h"
#include <iostream>
#include <iomanip>
#include <cmath>

int main() {
    // 尝试加载mnist-fc-plus模型
    std::cout << "Loading mnist-fc-plus model...\n";
    try {
        Model model = ModelLoader::loadModel("mnist-fc-plus");
        
        // 测试：输入全白(1)
        std::cout << "\n=== Test: All white (1) with mnist-fc-plus ===\n";
        Matrix allWhite(1, 784, 1.0f);
        std::vector<float> prob = model.forward(allWhite);
        for (size_t i = 0; i < prob.size(); i++) {
            std::cout << i << ": " << std::fixed << std::setprecision(4) << prob[i] << "\n";
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
    }
    
    return 0;
}
