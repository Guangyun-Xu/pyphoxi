cmake_minimum_required (VERSION 2.8.12)

project (pyphoxi)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y -pthread")

include_directories ("${PROJECT_SOURCE_DIR}/Server")
add_subdirectory (Server)

find_package(PhoXi REQUIRED CONFIG PATHS "$ENV{PHOXI_CONTROL_PATH}")

add_executable (pyphoxi phoxi.cpp)
target_link_libraries (pyphoxi Server)
target_link_libraries (pyphoxi ${PHOXI_LIBRARY} rt)
target_include_directories(pyphoxi PUBLIC ${PHOXI_INCLUDE_DIRS})