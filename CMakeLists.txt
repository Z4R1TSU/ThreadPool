# 设定CMake的最低版本要求
cmake_minimum_required(VERSION 3.10)

# 设置项目名称
project(MyThreadPool)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)

# 添加所有源文件到变量SOURCE
set(SOURCES
    test.cpp
    threadpool.cpp
)

# 创建一个名为test的可执行文件
add_executable(test ${SOURCES})

# 如果ThreadPool类有相关的头文件路径或者要链接的库，用下面的命令指定
# target_include_directories(test PRIVATE path/to/headers)
# target_link_libraries(test PRIVATE library_name)