cmake_minimum_required(VERSION 3.28)
project(TextureFontCreator_Project)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR})

find_package (ICU 4.4 REQUIRED)
FIND_PACKAGE (Qt5Widgets REQUIRED)
find_package (Freetype 2.3 REQUIRED)

include_directories(${FREETYPE_INCLUDE_DIRS} ${ICU_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR}/src)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

add_subdirectory(src)
