CMAKE_MINIMUM_REQUIRED(VERSION 3.10)
PROJECT(img_equalization VERSION 1.10.1 LANGUAGES CXX)
ENABLE_LANGUAGE(CXX)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS_DEBUG "-ggdb3 -O0 -Wall")
set(CMAKE_CXX_FLAGS_RELEASE "-g -O3 -Wall")
enable_testing()

FIND_PACKAGE(OpenCV REQUIRED)
LINK_LIBRARIES(${OpenCV_LIBS})
include_directories("${OpenCV_INCLUDE_DIRS}")

add_executable(img_equalization img_equalization.cpp common_code.cpp
    common_code.hpp clahe.cpp clahe.hpp)

