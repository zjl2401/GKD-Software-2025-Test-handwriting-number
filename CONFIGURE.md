# CMake 配置说明

## 当前状态

CMake 配置需要以下组件：
1. ✅ OpenCV - 已找到在 `C:\Users\yaping\Downloads\opencv-4.12.0\build`
2. ✅ C++ 编译器 - TDM-GCC-64 (GNU 10.3.0)

## 安装编译器

### 选项 1：安装 Visual Studio（推荐）

1. 下载并安装 [Visual Studio Community](https://visualstudio.microsoft.com/)
2. 安装时选择 "使用 C++ 的桌面开发" 工作负载
3. 安装完成后，在 **Developer Command Prompt** 或 **Developer PowerShell** 中运行 CMake

### 选项 2：安装 MinGW-w64

1. 下载 [MinGW-w64](https://www.mingw-w64.org/downloads/) 或使用 [MSYS2](https://www.msys2.org/)
2. 将 MinGW 的 `bin` 目录添加到系统 PATH
3. 使用 MinGW Makefiles 生成器

## 配置步骤

### 方法 1：在 IDE 中配置（推荐）

**VS Code / Cursor:**
- CMakeLists.txt 已自动配置，会自动查找 OpenCV
- 如果 OpenCV 不在默认路径，可以在 CMakeLists.txt 中修改 `OpenCV_POSSIBLE_PATHS`
- 或者设置环境变量 `OpenCV_DIR` 指向 OpenCV 的 build 目录

**其他 IDE:**
- CMakeLists.txt 会自动尝试查找 OpenCV
- 如果找不到，可以手动设置 `OpenCV_DIR` 变量

### 方法 2：手动配置

1. **清理 build 目录**（如果之前配置失败）：
```powershell
Remove-Item build\* -Recurse -Force
```

2. **使用 Visual Studio 生成器**（如果安装了 Visual Studio）：
```powershell
cd build
cmake -G "Visual Studio 17 2022" -A x64 -DOpenCV_DIR="C:/Users/yaping/Downloads/opencv-4.12.0/build" ..
```

3. **使用 MinGW 生成器**（如果安装了 MinGW）：
```powershell
cd build
cmake -G "MinGW Makefiles" -DOpenCV_DIR="C:/Users/yaping/Downloads/opencv-4.12.0/build" ..
```

4. **使用默认生成器**（需要 Visual Studio Build Tools）：
```powershell
cd build
cmake -DOpenCV_DIR="C:/Users/yaping/Downloads/opencv-4.12.0/build" ..
```

## 编译

配置成功后，运行：
```powershell
cmake --build .
```

或使用 Visual Studio 打开生成的 `.sln` 文件进行编译。

## 常见问题

### 找不到 OpenCV
- CMakeLists.txt 已自动配置，会自动查找常见路径的 OpenCV
- 如果 OpenCV 不在默认路径，可以：
  1. 修改 CMakeLists.txt 中的 `OpenCV_POSSIBLE_PATHS` 添加你的路径
  2. 设置环境变量 `OpenCV_DIR` 指向 OpenCV 的 build 目录
  3. 在 CMake 配置时使用 `-DOpenCV_DIR=路径` 参数
- 确保路径指向 `build` 目录（包含 `OpenCVConfig.cmake`）
- 使用正斜杠 `/` 或双反斜杠 `\\` 作为路径分隔符

### 找不到编译器
- 安装 Visual Studio 或 MinGW
- 在 Developer Command Prompt 中运行 CMake
- 或将编译器添加到系统 PATH

### 生成器不匹配
- 删除 `build` 目录中的所有文件
- 重新运行 CMake 配置



