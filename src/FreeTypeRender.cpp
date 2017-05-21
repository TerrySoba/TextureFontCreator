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

FreeTypeRender::FreeTypeRender(const std::string& fontpath, double fontSize) {
	// now init Freetype2
	int error = FT_Init_FreeType(&library);

	if (error) {
		std::cerr << "Could not initialize Freetype2: error #" << error	<< std::endl;
	}

	// now load font face
	error = FT_New_Face(library, fontpath.c_str(), 0, &face);
	if (error == FT_Err_Unknown_File_Format) {
		std::cerr << "Font file is in unsupported format." << std::endl;
	} else if (error) {
		std::cerr << "Font file could not be read." << std::endl;
	}
	error = FT_Set_Pixel_Sizes(face,      /* handle to face object */
	                         	 0,         /* pixel_width */
	                         	 fontSize); /* pixel_height */

	if (error) {
		std::cerr << "Could not set font size." << std::endl;
	}
}

FreeTypeRender::~FreeTypeRender() {
	// free memory
	FT_Done_FreeType(library);
}

std::shared_ptr<ImageCharacter> FreeTypeRender::renderUnicodeCharacter(uint32_t character) {
	int error = FT_Load_Char(face, character, FT_LOAD_RENDER);
	if (error) {
		std::cerr << "Could not load character: " << character << std::endl;
	}

	std::shared_ptr<GrayImage> image(new GrayImage(face->glyph->bitmap));

	std::shared_ptr<ImageCharacter> imgCharacter(new ImageCharacter());
	imgCharacter->image = image;
	imgCharacter->horiAdvance = face->glyph->linearHoriAdvance / (double)65536;
	imgCharacter->vertAdvance = face->glyph->linearVertAdvance / (double)65536;
	imgCharacter->bitmap_left = face->glyph->bitmap_left;
	imgCharacter->bitmap_top = face->glyph->bitmap_top;
	imgCharacter->unicode = character;

	return imgCharacter;
}

std::string FreeTypeRender::getFontName() {
	std::string name;
	if (face->family_name) {
		name += face->family_name;
	}

	if (face->style_name) {
		name += std::string(" ") + face->style_name;
	}

	if (name.size() == 0) {
		name = "(none)";
	}

	return name;
}
