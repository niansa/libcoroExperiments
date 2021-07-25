cmake_minimum_required(VERSION 3.5)

project(libcoroExperiments LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(libcoroExperiments main.cpp httpclient.cpp)

target_include_directories(libcoroExperiments PRIVATE .)

include(FetchContent)
FetchContent_Declare(
    libcoro
    GIT_REPOSITORY https://github.com/jbaldwin/libcoro.git
    GIT_TAG        475bcf6d8b1385ccb0e2501f73192dcbcb56a91f
)
FetchContent_MakeAvailable(libcoro)

target_link_libraries(libcoroExperiments PUBLIC libcoro)
