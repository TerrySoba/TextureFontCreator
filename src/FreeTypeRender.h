/*
 * FreeTypeRender.h
 *
 *  Created on: 24.03.2012
 *      Author: yoshi252
 */

#ifndef FREETYPERENDER_H_
#define FREETYPERENDER_H_

#include <QImage>
#include <string>
#include <memory>
#include <stdint.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include "GrayImage.h"

/*! \brief Information about a single rendered character
 *
 *  This structure is used to pass a single rendered character in
 *  combination with information on how to use the character in a
 *  font.
 */
struct ImageCharacter {
	std::shared_ptr<GrayImage> image;  //!< Pointer to the image of the character
	int32_t bitmap_left; //!< bearing from top of the bitmap
	int32_t bitmap_top; //!< bearing from left of the bitmap
	double horiAdvance; //!< horizontal advance of character
	double vertAdvance; //!< vertical advance of character
	uint32_t unicode; //!< unicode codepoint of this character
};

/*! \brief Wrapper to the FreeType2 Library
 *
 *  This class provides the means to open arbitrary TrueType fonts and render
 *  images of single characters from the font. To address a certain character
 *  the unicode codepoint of the character is needed.
 */
class FreeTypeRender {
public:
	/*! \brief Constructor
	 *
	 *  \param fontpath the path to the TrueType font
	 *  \param fontSize size of the font in pixels
	 */
	FreeTypeRender(const std::string& fontpath, double fontSize);
	virtual ~FreeTypeRender();

	/*! \brief renders a single character
	 *
	 *  The character is rendered into a greyscale image. It is returned in
	 *  an ImageCharacter structure that also contains information on how to
	 *  use the character as a font.
	 *
	 *  \param character unicode point to render
	 *  \return an ImageCharacter structure of the character
	 */
	std::shared_ptr<ImageCharacter> renderUnicodeCharacter(uint32_t character);

	std::string getFontName();

private:
	FT_Library library;
	FT_Face face;
};

#endif /* FREETYPERENDER_H_ */
