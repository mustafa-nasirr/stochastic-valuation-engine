cmake_minimum_required(VERSION 3.16)
project(valuation_engine CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

add_executable(valuation
    src/main.cpp
    src/DcfModel.cpp
    src/MonteCarloEngine.cpp
    src/Statistics.cpp
    src/DataLoader.cpp
    src/CompsAnalyzer.cpp
    src/Reporter.cpp
)
target_include_directories(valuation PRIVATE include)
