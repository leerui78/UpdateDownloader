cmake_minimum_required(VERSION 3.16)

project(UpdateDownloader VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ============================================
# 引入 Qt 模块（Widgets + Network）
# ============================================
find_package(Qt6 REQUIRED COMPONENTS Widgets Network)

set(PROJECT_SOURCES
    main.cpp
)

add_executable(UpdateDownloader ${PROJECT_SOURCES})

target_link_libraries(UpdateDownloader Qt::Widgets Qt::Network)

# ============================================
# 🔧 去掉黑色命令框（MinGW 专用）
# ============================================
if(WIN32)
    target_link_options(UpdateDownloader PRIVATE -Wl,-subsystem,windows)
endif()

# ============================================
# 自动部署 Qt 运行时（Release 模式下）
# ============================================
if(WIN32 AND CMAKE_BUILD_TYPE STREQUAL "Release")
    set(QT_INSTALL_PATH "${CMAKE_PREFIX_PATH}")
    if(EXISTS "${QT_INSTALL_PATH}/bin/windeployqt")
        add_custom_command(TARGET UpdateDownloader POST_BUILD
            COMMAND "${QT_INSTALL_PATH}/bin/windeployqt"
                --no-compiler-runtime
                "$<TARGET_FILE:UpdateDownloader>"
            COMMENT "Copying Qt runtime files..."
        )
    endif()
endif()
