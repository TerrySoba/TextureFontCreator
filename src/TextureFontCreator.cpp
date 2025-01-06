/*
 * TextureFontCreator.cpp
 *
 *  Created on: 25.03.2012
 *      Author: yoshi252
 */

#include "TextureFontCreator.h"
#include <unicode/unistr.h>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <set>


class Bisecter {
public:
	Bisecter(bool forcePowerOfTwo) :
		m_lowerBound(1),
		m_upperBound(2),
		m_initPeriod(true),
		m_forcePowerOfTwo(forcePowerOfTwo)
	{
	}

	void notBigEnough() {
		if (m_initPeriod) {
			m_lowerBound *= 2;
			m_upperBound *= 2;
		} else {
			if (m_upperBound - m_lowerBound < 2) {
				m_lowerBound = m_upperBound;
			} else {
				m_lowerBound = getValue();
			}
		}
	}

	void bigEnough() {
		m_upperBound = getValue();
		m_initPeriod = false;
	}

	int32_t getValue() {
		if (m_initPeriod || m_forcePowerOfTwo) {
			return m_upperBound;
		} else {
			return (m_lowerBound + m_upperBound) / 2;
		}
	}

	bool isDone() {
		if (m_forcePowerOfTwo && !m_initPeriod) {
			return true;
		}
		return (m_lowerBound == m_upperBound);
	}

private:
	int32_t m_lowerBound;
	int32_t m_upperBound;
	bool m_initPeriod;
	bool m_forcePowerOfTwo;
};


TextureFontCreator::TextureFontCreator(const std::string& fontpath, double fontSize, bool forcePowerOfTwoSize, const std::string& chars, bool enableAntiAliasing, bool enableHinting) {
	FreeTypeRender renderer(fontpath, fontSize, enableAntiAliasing, enableHinting);
	m_fontName = renderer.getFontName();
	icu::UnicodeString str(chars.c_str(), "UTF-8");

	// eliminate duplicates by putting the characters in a set
	std::set<uint32_t> characterSet;
	for (int32_t pos = 0; pos < str.length(); pos++) {
		characterSet.insert(str.char32At(pos));
	}

	for (uint32_t unicode : characterSet) {
		std::shared_ptr<ImageCharacter> imgChar = renderer.renderUnicodeCharacter(unicode);
		ImageOffset imgOff;
		imgOff.imgChar = imgChar;
		m_imageCharacters.push_back(imgOff);
	}

	stable_sort(m_imageCharacters.begin(), m_imageCharacters.end(), [](const ImageOffset& a, const ImageOffset& b) {
		return (a.imgChar->image->getHeight() < b.imgChar->image->getHeight());
	});

	Bisecter imageSize(forcePowerOfTwoSize);

	// determine required size
	while (!imageSize.isDone()) {
		int32_t size = imageSize.getValue();

		int32_t top = 0;
		int32_t left = 0;
		uint32_t max_height = 0;
		bool imageIsBigEnough = true;
		for (ImageOffset& imgOff : m_imageCharacters) {
			// now put imgOff.imgChar into image and increase top and/or left
			if (imgOff.imgChar->image->getWidth() + left
					>= size) {
				// image did not fit in line, use next line
				top += max_height + 1;
				left = 0;
				max_height = 0;
			}

			if (imgOff.imgChar->image->getHeight() + top
					>= size) {
				// image is too small to hold this font. We need to increase the texture size.
				imageSize.notBigEnough();
				imageIsBigEnough = false;
				break;
			}

			imgOff.top = top;
			imgOff.left = left;

			left += imgOff.imgChar->image->getWidth() + 1;
			if (imgOff.imgChar->image->getHeight() > max_height)
				max_height = imgOff.imgChar->image->getHeight();
		}

		if (imageIsBigEnough) {
			imageSize.bigEnough();
		}
	}

	// create image with font
	{
		m_image = std::shared_ptr<GrayImage> (new GrayImage(imageSize.getValue(), imageSize.getValue()));

		int32_t top = 0;
		int32_t left = 0;
		uint32_t max_height = 0;
		for (ImageOffset& imgOff : m_imageCharacters) {
			// now put imgOff.imgChar into image and increase top and/or left
			if (imgOff.imgChar->image->getWidth() + left
					>= m_image->getWidth()) {
				// image did not fit in line, use next line
				top += max_height + 1;
				left = 0;
				max_height = 0;
			}

			imgOff.top = top;
			imgOff.left = left;
			m_image->blit(*(imgOff.imgChar->image), left, top);

			left += imgOff.imgChar->image->getWidth() + 1;
			if (imgOff.imgChar->image->getHeight() > max_height)
				max_height = imgOff.imgChar->image->getHeight();
		}
	}
}

/**
 * Writes a plain old data type to a stream.
 *
 * \param [out] stream The stream to write to.
 * \param [in] data The data to be written.
 */
template <typename T>
void writeToStream(std::ostream& stream, const T& data)
{
	stream.write(reinterpret_cast<const char*>(&data), sizeof(T));
}

bool TextureFontCreator::writeToFile(const std::string& path) {
	// This code was only tested on little endian sytems.
	// If not otherwise specified all values are little endian.

	std::fstream fp(path, std::fstream::out | std::fstream::binary);
	if (fp.fail()) {
		std::cerr << "Could not open file \"" << path << "\" for writing. Aborting..." << std::endl;
		return false;
	}

	std::string fileSignature = "ytf252";

	fp.write(fileSignature.data(), fileSignature.size());

    uint16_t formatVersion = 4;
	writeToStream(fp, formatVersion);

	// write font name
	uint32_t fontNameLength = m_fontName.size();
	writeToStream(fp, fontNameLength);
	fp.write(m_fontName.data(), fontNameLength);

	uint32_t width  = m_image->getWidth();
	uint32_t height = m_image->getHeight();
	writeToStream(fp, width);  // write width of image
	writeToStream(fp, height); // write height of image

	{
        GrayImage imageCopy(*m_image);
		for (uint32_t row = 0; row < imageCopy.getHeight(); row++) { // write image to file
			fp.write(reinterpret_cast<const char*>(imageCopy.getRow(row)),
					 imageCopy.getWidth());
		}
	}

	uint32_t noOfCharacters = m_imageCharacters.size();
	writeToStream(fp, noOfCharacters); // write number of characters
	for (ImageOffset& imgOff : m_imageCharacters) {
		writeToStream(fp, imgOff.imgChar->unicode); // write unicode codepoint of character
		writeToStream(fp, imgOff.imgChar->bitmap_left); // write left bearing of character
		writeToStream(fp, imgOff.imgChar->bitmap_top); // write top bearing of character
		writeToStream(fp, imgOff.imgChar->horiAdvance); // horizontal advance of character
		writeToStream(fp, imgOff.imgChar->vertAdvance); // vertical advance of character
		writeToStream(fp, imgOff.left); // left offset of character in image
		writeToStream(fp, imgOff.top); // top offset of character in image

		uint32_t charWidth = imgOff.imgChar->image->getWidth();
		uint32_t charHeight = imgOff.imgChar->image->getHeight();

		writeToStream(fp, charWidth); // width of character
		writeToStream(fp, charHeight); // height of character
	}

	return true;
}


TextureFontCreator::~TextureFontCreator() {
	// TODO Auto-generated destructor stub
}

