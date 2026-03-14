# 构建说明

## 依赖
- CMake 3.14+
- C++17 编译器
- OpenCV 4.x
- nlohmann/json (可通过 vcpkg 或 CMake FetchContent 自动获取)

## 使用 vcpkg 构建
```bash
vcpkg install nlohmann-json opencv4
cmake -B build -DCMAKE_TOOLCHAIN_FILE=[vcpkg路径]/scripts/buildsystems/vcpkg.cmake
cmake --build build
```

## 使用系统库构建
若已安装 OpenCV，CMake 会自动通过 FetchContent 下载 nlohmann/json：
```bash
cmake -B build
cmake --build build
```

## 运行
- 图形界面（默认）：`./mnist_demo` 或 `./mnist_demo plus`（使用 mnist-fc-plus）
- Socket 服务端：`./mnist_demo --server` 或 `./mnist_demo plus --server`
- 连接远程 Socket：`./mnist_demo --socket 127.0.0.1`
- 测试 nums 图片：`./mnist_test`
