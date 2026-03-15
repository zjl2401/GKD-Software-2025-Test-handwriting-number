# 构建与运行

**依赖**：CMake 3.10+、C++17、OpenCV、nlohmann/json（FetchContent 自动拉取）

**构建**（在项目根目录执行）：
```powershell
.\configure_and_build.bat "C:\opencv\build"
```
或手动：`cmake -B build` → `cmake --build build`。可执行文件在 `build\Release\` 或 `build\Debug\`。

**运行**：双击 `run.bat` 或执行 `build\Release\HandwritingNumberRecognition.exe`。若 OpenCV 不在 PATH：`.\run.bat "C:\path\to\opencv\build"`。

| 命令 | 说明 |
|------|------|
| 无参数 | 图形界面，mnist-fc |
| `plus` | 图形界面，mnist-fc-plus |
| `--server` | Socket 服务端 |
| `--socket 127.0.0.1` | 图形界面，远程推理 |

操作：左侧画布绘制，右侧结果；`c` 清空，`q`/ESC 退出。

**故障排除**：找不到 OpenCV → 安装后指定含 `OpenCVConfig.cmake` 的目录，或运行 `install_opencv.bat`。找不到模型 → 将 `mnist-fc` 放到 exe 同目录。MSB1009 → 先执行 `cmake -B build` 再编译。
