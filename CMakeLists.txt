cmake_minimum_required(VERSION 3.10)
project(HandwritingNumberRecognition)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# MSVC: 使用 UTF-8 避免 C4819 警告
if(MSVC)
    add_compile_options(/utf-8)
endif()

# 包含项目自身的头文件目录
include_directories(${PROJECT_SOURCE_DIR}/include)

# nlohmann/json: 优先 find_package (vcpkg)，否则 FetchContent
find_package(nlohmann_json QUIET)
if(NOT nlohmann_json_FOUND)
    include(FetchContent)
    FetchContent_Declare(json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.11.2
    )
    FetchContent_MakeAvailable(json)
endif()

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

# 若已指定 OpenCV_DIR 但该目录下无 OpenCVConfig.cmake，则尝试子目录或备用路径
if(DEFINED OpenCV_DIR AND NOT EXISTS "${OpenCV_DIR}/OpenCVConfig.cmake")
    get_filename_component(_opencv_abs "${OpenCV_DIR}" REALPATH)
    if(NOT _opencv_abs STREQUAL OpenCV_DIR)
        set(OpenCV_DIR "${_opencv_abs}" CACHE PATH "OpenCV directory" FORCE)
    endif()
    # 常见子目录（预编译包）
    set(_opencv_subdirs "x64/vc16" "x64/vc15" "x64/vc14" "installed")
    foreach(_sub IN LISTS _opencv_subdirs)
        if(EXISTS "${OpenCV_DIR}/${_sub}/OpenCVConfig.cmake")
            set(OpenCV_DIR "${OpenCV_DIR}/${_sub}" CACHE PATH "OpenCV directory" FORCE)
            message(STATUS "Using OpenCV from subdir: ${OpenCV_DIR}")
            break()
        endif()
    endforeach()
    # 若解压后多一层 opencv（如 C:/opencv/opencv/build），尝试上一级下的 opencv/build
    if(NOT EXISTS "${OpenCV_DIR}/OpenCVConfig.cmake")
        get_filename_component(_parent "${OpenCV_DIR}" DIRECTORY)
        if(EXISTS "${_parent}/opencv/build/OpenCVConfig.cmake")
            set(OpenCV_DIR "${_parent}/opencv/build" CACHE PATH "OpenCV directory" FORCE)
            message(STATUS "Using OpenCV at: ${OpenCV_DIR}")
        endif()
    endif()
    # 递归搜索 OpenCVConfig.cmake
    if(NOT EXISTS "${OpenCV_DIR}/OpenCVConfig.cmake")
        file(GLOB_RECURSE _opencv_cfg "${OpenCV_DIR}/OpenCVConfig.cmake")
        list(LENGTH _opencv_cfg _n)
        if(_n GREATER 0)
            list(GET _opencv_cfg 0 _first)
            get_filename_component(OpenCV_DIR "${_first}" DIRECTORY)
            set(OpenCV_DIR "${OpenCV_DIR}" CACHE PATH "OpenCV directory" FORCE)
            message(STATUS "Found OpenCVConfig.cmake at: ${OpenCV_DIR}")
        else()
            if(EXISTS "${OpenCV_DIR}")
                message(FATAL_ERROR "OpenCVConfig.cmake not found under OpenCV_DIR: ${OpenCV_DIR}")
            else()
                message(FATAL_ERROR "OpenCV_DIR does not exist: ${OpenCV_DIR}")
            endif()
        endif()
    endif()
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

# 链接 OpenCV、nlohmann_json，Windows 需 ws2_32
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS} nlohmann_json::nlohmann_json)
if(WIN32)
    target_link_libraries(${PROJECT_NAME} ws2_32)
endif()

# Socket 客户端测试程序
add_executable(socket_client_test src/socket_client_test.cpp)
target_include_directories(socket_client_test PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(socket_client_test PRIVATE nlohmann_json::nlohmann_json)
if(WIN32)
    target_link_libraries(socket_client_test PRIVATE ws2_32)
endif()

add_executable(verify_model_nums src/verify_model_nums.cpp)
target_include_directories(verify_model_nums PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(verify_model_nums PRIVATE ${OpenCV_LIBS} nlohmann_json::nlohmann_json)

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
