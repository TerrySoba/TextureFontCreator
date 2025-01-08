cmake_minimum_required(VERSION 3.25)
project(TextureFontCreator)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR})

find_package(Qt6 REQUIRED COMPONENTS Core Widgets UiTools)
find_package(Freetype 2.3 REQUIRED)
find_package(nlohmann_json 3.11.0 REQUIRED)

qt_standard_project_setup()
set(CMAKE_AUTORCC ON)

include_directories(${FREETYPE_INCLUDE_DIRS})

qt_add_executable(
    ${PROJECT_NAME}
    src/main.cpp
    src/FreeTypeRender.h
    src/FreeTypeRender.cpp
    src/GrayImage.h
    src/GrayImage.cpp
    src/TextureFontCreator.h
    src/TextureFontCreator.cpp
    src/texturefontcreatorgui.cpp
    src/character_sets.h
    src/texturefontcreatorgui.h
    src/texturefontcreatorgui.ui
    src/graphics.qrc
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Widgets Qt6::Core Qt6::UiTools
    ${FREETYPE_LIBRARIES})