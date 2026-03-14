# 编译说明

## ⚠️ 重要提示

由于 Windows 上的 `mingw32-make` 工具存在问题，**请勿使用 CMake 扩展的构建按钮**。

## ✅ 推荐编译方法

### 方法 1：使用 IDE 任务（最简单）

1. 按快捷键 `Ctrl+Shift+B`
2. 选择 "Build with build.ps1" 任务
3. 等待编译完成

### 方法 2：使用终端

在项目根目录运行：

```powershell
.\build.ps1
```

### 方法 3：使用 CMake 自定义目标

如果必须使用 CMake，可以运行：

```powershell
cd build
cmake --build . --target build_with_script
```

## 📁 编译输出

编译成功后，可执行文件位于：
- `build/HandwritingNumberRecognition.exe`
- 所有必要的 OpenCV DLL 文件也会自动复制到 `build/` 目录

## 🚫 避免使用

- ❌ CMake 扩展的构建按钮（会导致 mingw32-make 错误）
- ❌ 直接运行 `cmake --build .`（会使用有问题的 make 工具）

## 🔧 故障排除

如果遇到编译问题：

1. 确保 OpenCV 已正确编译（库文件存在于 `C:/Users/yaping/Downloads/opencv-4.12.0/build/lib/`）
2. 检查编译器路径是否正确（`C:/TDM-GCC-64/bin/g++.exe`）
3. 运行 `.\build.ps1` 查看详细错误信息

