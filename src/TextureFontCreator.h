/*
 * TextureFontCreator.h
 *
 *  Created on: 25.03.2012
 *      Author: yoshi252
 */

#ifndef TEXTUREFONTCREATOR_H_
#define TEXTUREFONTCREATOR_H_

#include <stdint.h>
#include <vector>
#include <memory>
#include "GrayImage.h"
#include "FreeTypeRender.h"

struct ImageOffset {
	std::shared_ptr<ImageCharacter> imgChar;
	int32_t left;
	int32_t top;
};

class TextureFontCreator {
public:
	virtual ~TextureFontCreator();
	TextureFontCreator(const std::string& fontpath, double fontSize, bool forcePowerOfTwoSize, const std::string& chars);

	std::shared_ptr<GrayImage> getImage() { return m_image; }

	bool writeToFile(const std::string& path);

	std::string getFontName() { return m_fontName; }

private:
	std::shared_ptr<GrayImage> m_image;
	std::vector<ImageOffset> m_imageCharacters;
	std::string m_fontName;
};

#endif /* TEXTUREFONTCREATOR_H_ */
