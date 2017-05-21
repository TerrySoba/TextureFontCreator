cmake_minimum_required(VERSION 2.8.11)
project(TextureFontCreator)

SET(
    ${PROJECT_NAME}_SOURCES
    FreeTypeRender.h
    FreeTypeRender.cpp
    GrayImage.h
    GrayImage.cpp
    TextureFontCreator.h
    TextureFontCreator.cpp
    texturefontcreatorgui.cpp
    character_sets.h
    texturefontcreatorgui.h
)

SET(
    ${PROJECT_NAME}_HEADERS 
    texturefontcreatorgui.h
)

SET(
    ${PROJECT_NAME}_FORMS 
    texturefontcreatorgui.ui
)

SET(
    ${PROJECT_NAME}_RESOURCES 
    graphics.qrc
)

QT5_WRAP_UI(${PROJECT_NAME}_FORMS_HEADERS ${${PROJECT_NAME}_FORMS})
QT5_ADD_RESOURCES(${PROJECT_NAME}_RESOURCES_RCC ${${PROJECT_NAME}_RESOURCES})

add_library(
    ${PROJECT_NAME} SHARED
    ${${PROJECT_NAME}_SOURCES} 
    ${${PROJECT_NAME}_HEADERS_MOC} 
    ${${PROJECT_NAME}_FORMS_HEADERS} 
    ${${PROJECT_NAME}_RESOURCES_RCC}
)

target_link_libraries(${PROJECT_NAME} ${ICU_LIBRARIES} ${FREETYPE_LIBRARIES} Qt5::Widgets)

add_executable(
    ${PROJECT_NAME}Gui
    main.cpp
)

target_link_libraries(${PROJECT_NAME}Gui ${PROJECT_NAME})
