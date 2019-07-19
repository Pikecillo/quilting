cmake_minimum_required(VERSION 2.8)
project(Quilting)

find_package(OpenCV REQUIRED)

set(QUILTING_SRC_DIR src)

file(GLOB QUILTING_SRC ${QUILTING_SRC_DIR}/*.cpp)

include_directories(${QUILTING_SRC_DIR})

add_executable(synthesis synthesis.cpp ${QUILTING_SRC})

target_link_libraries(synthesis ${OpenCV_LIBS})
