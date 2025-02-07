/*
 * TextureFontCreator.cpp
 *
 *  Created on: 25.03.2012
 *      Author: yoshi252
 */

#include "TextureFontCreator.h"

#include <algorithm>
#include <iostream>
#include <fstream>
#include <set>
#include <sstream>
#include <filesystem>
#include <iconv.h>

#include <nlohmann/json.hpp>
#include <QBuffer>



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



static std::u32string toU32String(const std::u8string& str) {
    std::u32string result;
    for (size_t i = 0; i < str.size(); ) {
        char32_t ch = 0;
        uint8_t byte = str.at(i);
        if (byte < 0x80) {
            ch = byte;
            ++i;
        } else if ((byte & 0xE0) == 0xC0) {
            ch = ((byte & 0x1F) << 6) | (str.at(i + 1) & 0x3F);
            i += 2;
        } else if ((byte & 0xF0) == 0xE0) {
            ch = ((byte & 0x0F) << 12) | ((str.at(i + 1) & 0x3F) << 6) | (str.at(i + 2) & 0x3F);
            i += 3;
        } else if ((byte & 0xF8) == 0xF0) {
            ch = ((byte & 0x07) << 18) | ((str.at(i + 1) & 0x3F) << 12) | ((str.at(i + 2) & 0x3F) << 6) | (str.at(i + 3) & 0x3F);
            i += 4;
        } else {
            throw std::runtime_error("Invalid UTF-8 sequence.");
        }
        result.push_back(ch);
    }
    return result;
}


TextureFontCreator::TextureFontCreator(
    const std::filesystem::path& fontpath,
    double fontSize,
    bool forcePowerOfTwoSize,
    const std::u8string& chars,
    bool enableAntiAliasing,
    bool enableHinting)
{

    FreeTypeRender renderer(fontpath, fontSize, enableAntiAliasing, enableHinting);
    m_fontName = renderer.getFontName();
  
    std::u32string str = toU32String(chars);

    // eliminate duplicates by putting the characters in a set
    std::set<char32_t> characterSet;
    for (int32_t pos = 0; pos < str.length(); pos++) {
        characterSet.insert(str.at(pos));
    }

    for (char32_t unicode : characterSet) {
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

void TextureFontCreator::writeToFile(const std::filesystem::path& path) {
    // This code was only tested on little endian systems.
    // If not otherwise specified all values are little endian.
    std::fstream fp(path, std::fstream::out | std::fstream::binary);
    if (fp.fail()) {
        std::stringstream errorText;
        errorText << "Could not open file \"" << path.native() << "\" for writing. Aborting...";
        throw std::runtime_error(errorText.str());
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

}


void TextureFontCreator::writeToJsonFile(const std::filesystem::path& path)
{
    nlohmann::json json;

    json["format"] = "ytf252";
    json["format_version"] = 4;
    json["font_name"] = m_fontName;

    json["image_width"] = m_image->getWidth();
    json["image_height"] = m_image->getHeight();

    auto qImage = m_image->getQImage();
    // now get PNG data base64 encoded
    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    buffer.open(QIODevice::WriteOnly);
    qImage->save(&buffer, "PNG");
    json["image_data_png"] = byteArray.toBase64().toStdString();
    json["characters"] = nlohmann::json::array();

    for (ImageOffset& imgOff : m_imageCharacters) {

        nlohmann::json character;

        character["unicode"] = imgOff.imgChar->unicode;
        character["bitmap_left"] = imgOff.imgChar->bitmap_left;
        character["bitmap_top"] = imgOff.imgChar->bitmap_top;
        character["hori_advance"] = imgOff.imgChar->horiAdvance;
        character["vert_advance"] = imgOff.imgChar->vertAdvance;
        character["left"] = imgOff.left;
        character["top"] = imgOff.top;
        character["width"] = imgOff.imgChar->image->getWidth();
        character["height"] = imgOff.imgChar->image->getHeight();

        json["characters"].push_back(character);

    }

    std::fstream fp(path, std::fstream::out | std::fstream::binary);
    if (fp.fail()) {
        std::stringstream errorText;
        errorText <<"Could not open file \"" << path.native() << "\" for writing. Aborting...";
        throw std::runtime_error(errorText.str());
    }

    fp << json.dump(4);
    fp.close();
}


class UTF32toCP437Converter {
public:
    UTF32toCP437Converter()
    {
        if ((m_cd = iconv_open("CP437", "UTF-32")) == (iconv_t)(-1))
        {
            throw std::runtime_error("Cannot open converter");
        }
    }

    ~UTF32toCP437Converter()
    {
        iconv_close(m_cd);
    }


    std::string convert(const std::u32string& str)
    {
        std::vector<char> inVec((const char*)str.data(), ((const char*)str.data()) + str.size() * 4);

        char* inptr = inVec.data();
        size_t inleft = inVec.size();

        std::vector<char> outVec(inVec.size());
        char* outptr = outVec.data();
        size_t outleft = outVec.size();

        int rc = iconv(m_cd, &inptr, &inleft, &outptr, &outleft);
        if (rc == -1)
        {
            throw std::runtime_error("Error in converting characters");
        }

        return std::string(outVec.data(), outVec.size() - outleft);
    }

private:
    iconv_t m_cd;
};

void TextureFontCreator::writeToSimpleFile(const std::filesystem::path& path)
{
    // This code was only tested on little endian systems.
    // If not otherwise specified all values are little endian.
    std::fstream fp(path, std::fstream::out | std::fstream::binary);
    if (fp.fail()) {
        std::stringstream errorText;
        errorText << "Could not open file \"" << path.native() << "\" for writing. Aborting...";
        throw std::runtime_error(errorText.str());
    }

    std::string fileSignature = "stf252";
    fp.write(fileSignature.data(), fileSignature.size());

    uint16_t formatVersion = 1;
    writeToStream(fp, formatVersion);

    uint16_t width  = m_image->getWidth();
    uint16_t height = m_image->getHeight();
    writeToStream(fp, width);  // write width of image
    writeToStream(fp, height); // write height of image

    {
        GrayImage imageCopy(*m_image);
        for (uint32_t row = 0; row < imageCopy.getHeight(); row++) { // write image to file
            fp.write(reinterpret_cast<const char*>(imageCopy.getRow(row)),
                     imageCopy.getWidth());
        }
    }

    uint16_t noOfCharacters = m_imageCharacters.size();
    writeToStream(fp, noOfCharacters); // write number of characters

    UTF32toCP437Converter converter;
    for (ImageOffset& imgOff : m_imageCharacters)
    {
        std::u32string in;
        in.push_back(imgOff.imgChar->unicode);
        std::string cp437 = converter.convert(in);
        writeToStream(fp, (uint8_t)cp437.at(0)); // write cp437 codepoint of character
        writeToStream(fp, (int16_t)imgOff.imgChar->bitmap_left); // write left bearing of character
        writeToStream(fp, (int16_t)imgOff.imgChar->bitmap_top); // write top bearing of character
        writeToStream(fp, (int16_t)ceil(imgOff.imgChar->horiAdvance)); // horizontal advance of character
        writeToStream(fp, (int16_t)ceil(imgOff.imgChar->vertAdvance)); // vertical advance of character
        writeToStream(fp, (int16_t)imgOff.left); // left offset of character in image
        writeToStream(fp, (int16_t)imgOff.top); // top offset of character in image

        uint32_t charWidth = imgOff.imgChar->image->getWidth();
        uint32_t charHeight = imgOff.imgChar->image->getHeight();

        writeToStream(fp, (int16_t)charWidth); // width of character
        writeToStream(fp, (int16_t)charHeight); // height of character
    }

}


TextureFontCreator::~TextureFontCreator() {
    // TODO Auto-generated destructor stub
}

enum class DrawStage
{
    CALCULATE_SIZE,
    DRAW_IMAGE
};

std::shared_ptr<GrayImage> TextureFontCreator::renderText(const std::u8string& text)
{
    std::u32string utf32 = toU32String(text);

    std::shared_ptr<GrayImage> result;

    for (DrawStage stage : {DrawStage::CALCULATE_SIZE, DrawStage::DRAW_IMAGE})
    {
        int32_t top = 0;
        int32_t left = 0;
        int32_t maxHeight = 0;
        for (char32_t unicode : utf32) {
            for (ImageOffset& imgOff : m_imageCharacters) {
                if (imgOff.imgChar->unicode == unicode) {
                    if (stage == DrawStage::DRAW_IMAGE)
                    {
                        result->blit(*(imgOff.imgChar->image), left + imgOff.imgChar->bitmap_left, ceil(imgOff.imgChar->vertAdvance) - imgOff.imgChar->bitmap_top);
                    }

                    left += ceil(imgOff.imgChar->horiAdvance);

                    auto height = ceil(imgOff.imgChar->vertAdvance) - imgOff.imgChar->bitmap_top + imgOff.imgChar->image->getHeight();

                    if (height > maxHeight)
                    {
                        maxHeight = height;
                    }
                        
                    break;
                }
            }
        }

        if (stage == DrawStage::CALCULATE_SIZE)
        {
            result = std::shared_ptr<GrayImage>(new GrayImage(left, maxHeight));
        }
    }
    

    return result;
}
