/*
 * GrayImage.cpp
 *
 *  Created on: 25.03.2012
 *      Author: yoshi252
 */

#include "GrayImage.h"

#include <iostream>
#include <sstream>

GrayImage::GrayImage(uint32_t width, uint32_t height, uint8_t fill) {
    data.resize(width * height, fill);
    this->width = width;
    this->rows = height;
    this->pitch = width;
}

GrayImage::GrayImage(FT_Bitmap& bitmap) {
    data.resize(bitmap.width * bitmap.rows);
    this->pitch = bitmap.width;
    this->width = bitmap.width;
    this->rows = bitmap.rows;

    switch(bitmap.pixel_mode) {
        case FT_PIXEL_MODE_MONO:
            for (int32_t row = 0; row < bitmap.rows; row++) {
                for (int32_t x = 0; x < bitmap.width; x++) {
                    data[row * pitch + x] = (bitmap.buffer[row * bitmap.pitch + x / 8] & (1 << (7 - x % 8))) ? 255 : 0;
                }
            }
            break;

        case FT_PIXEL_MODE_GRAY:
            for (int32_t row = 0; row < bitmap.rows; row++) {
                memcpy(data.data() + row * pitch, bitmap.buffer + row * bitmap.pitch, width);
            }
            break;

        default:
            // throw exception
            std::stringstream errorText;
            errorText << "FreeType delivered invalid pixel mode: " << static_cast<int32_t>(bitmap.pixel_mode);
            throw std::runtime_error(errorText.str());
    }
}

GrayImage::GrayImage(const GrayImage& other) {
    data.resize(other.width * other.rows);
    this->pitch = other.pitch;
    this->width = other.width;
    this->rows = other.rows;

    // now copy data
    memcpy(data.data(), other.data.data(), other.width * other.rows);
}

GrayImage::~GrayImage() {
    // TODO Auto-generated destructor stub
}

bool GrayImage::blit(GrayImage& source, uint32_t posX, uint32_t posY) {
    // check if source image fits in this image
    if (source.getWidth() + posX > this->getWidth()) {
        return false;
    }
    if (source.getHeight() + posY > this->getHeight()) {
        return false;
    }

    // now blit image
    for (uint32_t row = 0; row < source.getHeight(); row++) {
        memcpy(this->getRow(row + posY) + posX, source.getRow(row), source.getWidth());
    }

    return true;
}

/*! \brief flips an image vertically
 *
 *  \param ptr pointer to the first line of pixels
 *  \param pitch distance between the first pixel of a line to the first pixel of the next line in bytes
 *  \param lineLength length of a line in bytes
 *  \param lines the number of lines of the image
 */
void FlipVertically(void* ptr, size_t pitch, size_t lineLength, size_t lines) {
    std::vector<uint8_t> linebuffer(lineLength);
    for (size_t line = 0; line < lines / 2; line++) {
        memcpy(linebuffer.data(),
                       &(((uint8_t*) (ptr))[pitch * line]),
                       lineLength); // save line to buffer

        memcpy(&(((uint8_t*) (ptr))[pitch * line]),
                       &(((uint8_t*) (ptr))[pitch * (lines - line - 1)]),
                       lineLength); // overwrite old line

        memcpy(&(((uint8_t*) (ptr))[pitch * (lines - line - 1)]),
                       linebuffer.data(),
                       lineLength); // restore line from buffer
    }
}

void GrayImage::flipVertically() {
    FlipVertically(data.data(), pitch, width, rows);
}

std::shared_ptr<QImage> GrayImage::getQImage(uint8_t red, uint8_t green, uint8_t blue) {
    std::shared_ptr<QImage> image(new QImage(QSize(this->getWidth(), this->getHeight()), QImage::Format_ARGB32));

    for (uint32_t y = 0; y < getHeight(); y++) {
        uint32_t* line = reinterpret_cast<uint32_t*>(image->scanLine(y));
        uint8_t* sourceLine = getRow(y);
        for (uint32_t x = 0; x < getWidth(); x++) {
            uint32_t color = qRgb(sourceLine[x] * (double) red   / 255.0,
                                  sourceLine[x] * (double) blue  / 255.0,
                                  sourceLine[x] * (double) green / 255.0);
            line[x] = color;
        }
    }

    return image;
}



