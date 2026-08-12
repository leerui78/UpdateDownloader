cmake_minimum_required(VERSION 3.16)

project(MyApp)

set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 17)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

set(SOURCES main.cpp)

add_executable(MyApp ${SOURCES})
target_link_libraries(MyApp Qt::Widgets)

# ============================================
# MinGW 专用：去掉黑窗口
# ============================================
if(WIN32 AND MINGW)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--subsystem,windows")
endif()
