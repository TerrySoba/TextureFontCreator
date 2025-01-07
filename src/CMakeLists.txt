cmake_minimum_required(VERSION 3.25)
project(TextureFontCreator)

qt_add_executable(
    ${PROJECT_NAME}
    main.cpp
    FreeTypeRender.h
    FreeTypeRender.cpp
    GrayImage.h
    GrayImage.cpp
    TextureFontCreator.h
    TextureFontCreator.cpp
    texturefontcreatorgui.cpp
    character_sets.h
    texturefontcreatorgui.h
    texturefontcreatorgui.ui
    graphics.qrc
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Widgets Qt6::Core
    ${ICU_LIBRARIES} ${FREETYPE_LIBRARIES})
