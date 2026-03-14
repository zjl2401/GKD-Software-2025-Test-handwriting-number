# 构建说明

## 前置要求

1. **CMake** (版本 3.10 或更高)
2. **C++ 编译器** (支持 C++17 标准)
   - Windows: Visual Studio 2017 或更高版本，或 MinGW
   - Linux: GCC 7 或更高版本
   - macOS: Clang (Xcode Command Line Tools)
3. **OpenCV** (需要安装并配置)
4. **nlohmann/json** (通过 CMake FetchContent 自动下载，或 vcpkg 安装)

## 安装 OpenCV

### Windows
1. 从 [OpenCV官网](https://opencv.org/releases/) 下载预编译版本（例如 `opencv-4.12.0.zip`）
2. 解压到某个目录（例如 `C:\opencv` 或 `C:\opencv-4.12.0`）
   - 解压后应该有一个 `opencv` 文件夹，里面包含 `build` 和 `sources` 目录
3. 设置环境变量 `OpenCV_DIR` 指向 OpenCV 的 `build` 目录
   - 例如：`C:\opencv\opencv\build` 或 `C:\opencv-4.12.0\opencv\build`
   - 在 PowerShell 中临时设置：`$env:OpenCV_DIR = "C:\opencv\opencv\build"`
   - 或在系统环境变量中永久设置
4. 或者使用 vcpkg: `vcpkg install opencv`
5. 或者在 CMake 配置时直接指定路径（见下方构建步骤）

### Linux
```bash
sudo apt-get update
sudo apt-get install libopencv-dev
```

### macOS
```bash
brew install opencv
```

## 构建步骤

### 1. 创建构建目录
```bash
mkdir build
cd build
```

### 2. 配置 CMake
```bash
cmake ..
```

如果 OpenCV 不在标准路径，可以指定：
```bash
# Windows 示例（指向 build 目录）
cmake -DOpenCV_DIR=C:/opencv/opencv/build ..

# Linux/macOS 示例
cmake -DOpenCV_DIR=/path/to/opencv/build ..
```

**注意**：`OpenCV_DIR` 应该指向包含 `OpenCVConfig.cmake` 的目录，通常是解压后的 `opencv/build` 目录。

### 3. 编译
```bash
cmake --build .
```

或者使用：
- Windows (Visual Studio): 打开生成的 `.sln` 文件
- Linux/macOS: `make`

**Windows 用户**：若使用 MinGW Makefiles 遇到问题，可改用 `.\build.ps1` 脚本编译。

### 4. 运行
```bash
./HandwritingNumberRecognition
```

或者 Windows:
```bash
HandwritingNumberRecognition.exe
```

## 使用说明

1. 默认加载 `mnist-fc` 模型；加参数 `plus` 可加载 `mnist-fc-plus`（double 精度、更大 hidden）
   - 默认：`./HandwritingNumberRecognition` 或 `HandwritingNumberRecognition.exe`
   - Plus 版：`./HandwritingNumberRecognition plus` 或 `HandwritingNumberRecognition.exe plus`
2. 程序启动后会自动加载对应模型
3. 在左侧画布上用鼠标绘制数字（按住左键拖动）
4. 右侧会实时显示识别结果和概率分布
5. 按 `c` 键清空画布
6. 按 `q` 或 `ESC` 键退出程序

## 项目结构

```
.
├── CMakeLists.txt          # CMake 配置文件
├── include/                # 头文件目录（Header-only 库）
│   ├── matrix.h           # 矩阵类（完整实现）
│   ├── model.h            # 模型类（完整实现）
│   └── file_reader.h      # 文件读取工具（完整实现）
├── src/                   # 源文件目录
│   └── main.cpp           # 主程序
├── mnist-fc/              # 模型参数文件夹
│   ├── meta.json
│   ├── fc1.weight
│   ├── fc1.bias
│   ├── fc2.weight
│   └── fc2.bias
└── build/                 # 构建输出目录（需要创建）
```

## 故障排除

### OpenCV 找不到
- 确保 OpenCV 已正确解压（从 zip 文件解压后应该有 `opencv/build` 目录）
- 设置 `OpenCV_DIR` 环境变量指向 `opencv/build` 目录（不是 `opencv` 根目录）
- 或在 CMake 配置时指定路径：`cmake -DOpenCV_DIR=C:/opencv/opencv/build ..`
- 检查 `opencv/build` 目录下是否存在 `OpenCVConfig.cmake` 文件
- Windows 用户注意：路径中使用正斜杠 `/` 或双反斜杠 `\\`，例如 `C:/opencv/opencv/build`

### 运行时找不到模型文件
- 确保 `mnist-fc` 文件夹在可执行文件同一目录
- 或在代码中修改模型路径
