/*
 * FreeTypeRender.cpp
 *
 *  Created on: 24.03.2012
 *      Author: yoshi252
 */

#include "FreeTypeRender.h"

#include <cstdlib>
#include <cstring>
#include <stdint.h>
#include <iostream>
#include <unicode/unistr.h>
#include <sstream>

FreeTypeRender::FreeTypeRender(const std::filesystem::path& fontpath, double fontSize, bool enableAntiAliasing, bool enableHinting)
    : m_enableAntiAliasing(enableAntiAliasing), m_enableHinting(enableHinting)
{
    // now init Freetype2
    int error = FT_Init_FreeType(&m_library);
    
    if (error) {
        std::stringstream errorText;
        errorText << "Could not initialize Freetype2: error #" << error;
        throw std::runtime_error(errorText.str());
    }

    // now load font face
    error = FT_New_Face(m_library, reinterpret_cast<const char*>(fontpath.u8string().c_str()), 0, &m_face);
    if (error == FT_Err_Unknown_File_Format) {
        throw std::runtime_error("Font file is in unsupported format.");
    } else if (error) {
        throw std::runtime_error("Font file could not be read.");
    }
    error = FT_Set_Pixel_Sizes(m_face,    /* handle to face object */
                               0,         /* pixel_width */
                               fontSize); /* pixel_height */

    if (error) {
        throw std::runtime_error("Could not set font size.");
    }
}

FreeTypeRender::~FreeTypeRender() {
    // free memory
    FT_Done_FreeType(m_library);
}

std::shared_ptr<ImageCharacter> FreeTypeRender::renderUnicodeCharacter(uint32_t character) {

    int flags = FT_LOAD_RENDER;
    if (m_enableAntiAliasing) {
        flags |= FT_LOAD_TARGET_NORMAL;
    } else {
        flags |= FT_LOAD_TARGET_MONO;
    }

    if (!m_enableHinting) {
        flags |= FT_LOAD_NO_HINTING;
    }

    int error = FT_Load_Char(m_face, character, flags);
    if (error) {
        std::stringstream errorText;
        errorText << "Could not load character: " << character;
        throw std::runtime_error(errorText.str());
    }

    std::shared_ptr<GrayImage> image(new GrayImage(m_face->glyph->bitmap));

    std::shared_ptr<ImageCharacter> imgCharacter(new ImageCharacter());
    imgCharacter->image = image;
    imgCharacter->horiAdvance = m_face->glyph->linearHoriAdvance / (double)65536;
    imgCharacter->vertAdvance = m_face->glyph->linearVertAdvance / (double)65536;
    imgCharacter->bitmap_left = m_face->glyph->bitmap_left;
    imgCharacter->bitmap_top = m_face->glyph->bitmap_top;
    imgCharacter->unicode = character;

    return imgCharacter;
}

std::string FreeTypeRender::getFontName() {
    std::string name;
    if (m_face->family_name) {
        name += m_face->family_name;
    }

    if (m_face->style_name) {
        name += std::string(" ") + m_face->style_name;
    }

    if (name.size() == 0) {
        name = "(none)";
    }

    return name;
}
