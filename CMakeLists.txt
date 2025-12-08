cmake_minimum_required(VERSION 3.10)
project(HandwritingNumberRecognition)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 包含项目自身的头文件目录
include_directories(${PROJECT_SOURCE_DIR}/include)

# 设置OpenCV路径（如果未设置）
if(NOT OpenCV_DIR)
    # 尝试常见的OpenCV安装路径
    set(OpenCV_POSSIBLE_PATHS
        "C:/Users/yaping/Downloads/opencv-4.12.0/build"
        "C:/opencv/build"
        "C:/opencv-4.12.0/build"
        "$ENV{OpenCV_DIR}"
    )
    
    foreach(PATH ${OpenCV_POSSIBLE_PATHS})
        if(EXISTS "${PATH}/OpenCVConfig.cmake")
            set(OpenCV_DIR "${PATH}" CACHE PATH "OpenCV directory" FORCE)
            message(STATUS "Found OpenCV at: ${OpenCV_DIR}")
            break()
        endif()
    endforeach()
endif()

# 查找OpenCV（通过vcpkg或系统安装）
find_package(OpenCV REQUIRED COMPONENTS core imgproc highgui videoio)

# 源文件和头文件
set(SOURCES
    src/main.cpp
)

set(HEADERS
    include/matrix.h
    include/model.h
    include/file_reader.h
)

# 创建可执行文件
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# 链接OpenCV库
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS})

# ⚠️ 重要提示：由于 Windows 上 mingw32-make 工具存在问题
# 请使用 build.ps1 脚本编译，而不是使用 CMake 的构建命令
if(WIN32 AND CMAKE_GENERATOR MATCHES "Makefiles")
    message(WARNING "")
    message(WARNING "================================================")
    message(WARNING "⚠️  警告：检测到使用 Makefiles 生成器")
    message(WARNING "⚠️  mingw32-make 在 Windows 上存在问题")
    message(WARNING "⚠️  请使用以下方法编译：")
    message(WARNING "   1. 按 Ctrl+Shift+B 选择 'Build with build.ps1'")
    message(WARNING "   2. 或运行: .\build.ps1")
    message(WARNING "   3. 或运行: cmake --build . --target build_with_script")
    message(WARNING "================================================")
    message(WARNING "")
    
    # 添加自定义构建目标，使用 build.ps1 脚本
    add_custom_target(build_with_script
        COMMAND powershell -ExecutionPolicy Bypass -File "${CMAKE_SOURCE_DIR}/build.ps1"
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "使用 build.ps1 脚本编译项目（推荐方法）"
    )
endif()