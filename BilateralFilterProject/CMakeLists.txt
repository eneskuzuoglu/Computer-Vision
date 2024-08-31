cmake_minimum_required(VERSION 3.10)
project(BilateralFilterProject)

set(CMAKE_CXX_STANDARD 11)

# Eigen kütüphanesinin bulunduğu dizini ayarla
set(EIGEN3_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/C:/Users/Enesk/Desktop/coding/kivi/BilateralFilterProject/eigen-3.4.0")
include_directories(${EIGEN3_INCLUDE_DIR})

# Include dizinlerini ekle



# Kaynak dosyalarını belirt
add_executable(BilateralFilterProject
    src/main.cpp
)
