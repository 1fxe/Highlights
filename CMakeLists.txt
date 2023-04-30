cmake_minimum_required(VERSION 3.25)
project(MCHighlights)

set(CMAKE_CXX_STANDARD 11)

find_package(JNI REQUIRED)
include_directories(${JNI_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/include)
find_library(Gfe_SDK_LIBARY GfeSDK ${CMAKE_CURRENT_SOURCE_DIR}/lib)

add_library(MCHighlights SHARED GfeSDKWrapper.cpp library.cpp)
target_link_libraries(${PROJECT_NAME} ${Gfe_SDK_LIBARY})
