if(APPLE)
  set(CMAKE_OSX_DEPLOYMENT_TARGET "27.0" CACHE STRING "Minimum macOS version")
endif()

cmake_minimum_required(VERSION 3.16)
project(otel_learning)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(control)
