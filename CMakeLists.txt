cmake_minimum_required(VERSION 3.14)
project(mnist_fc LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# nlohmann-json: prefer find_package (vcpkg), else FetchContent
find_package(nlohmann_json QUIET)
if(NOT nlohmann_json_FOUND)
    include(FetchContent)
    FetchContent_Declare(json GIT_REPOSITORY https://github.com/nlohmann/json.git GIT_TAG v3.11.2)
    FetchContent_MakeAvailable(json)
endif()
find_package(OpenCV REQUIRED)

add_executable(mnist_demo
    src/main.cpp
)
target_include_directories(mnist_demo PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(mnist_demo PRIVATE
    nlohmann_json::nlohmann_json
    ${OpenCV_LIBS}
)
if(WIN32)
    target_link_libraries(mnist_demo PRIVATE ws2_32)
endif()

add_executable(mnist_test src/test_nums.cpp)
target_include_directories(mnist_test PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(mnist_test PRIVATE nlohmann_json::nlohmann_json ${OpenCV_LIBS})
if(WIN32)
    target_link_libraries(mnist_test PRIVATE ws2_32)
endif()
