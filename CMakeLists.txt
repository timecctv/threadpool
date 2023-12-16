cmake_minimum_required(VERSION 3.0)
project(threadpool)

set(CMAKE_CXX_FLAGS $(CMAKE_CXX_FLAGS) -g)

include_directories($(PROJECT_SOURCE_DIR)/include)

add_subdirectory(src)