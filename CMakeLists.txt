cmake_minimum_required(VERSION 3.25)
project(TextureFontCreator_Project)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR})

find_package (ICU 4.4 REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)
find_package (Freetype 2.3 REQUIRED)
find_package (nlohmann_json 3.11.0 REQUIRED)

qt_standard_project_setup()

include_directories(${FREETYPE_INCLUDE_DIRS} ${ICU_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR}/src)

add_subdirectory(src)