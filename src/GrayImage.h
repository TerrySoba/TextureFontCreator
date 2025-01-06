/*
 * GrayImage.h
 *
 *  Created on: 25.03.2012
 *      Author: yoshi252
 */

#ifndef GRAYIMAGE_H_
#define GRAYIMAGE_H_

#include <stdint.h>
#include <memory>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <QImage>


/*! \brief Gray scale image class
 *
 *  This class provides means to store and manipulate grayscale images.
 *  These image always use a single 8bit value per pixel.
 *  It is possible to construct the image from a truetype2 FT_Bitmap.
 *  It is also possible to export the image as a QImage.
 *
 *  As Qt is a very heavy dependency support for exporting to QImage is
 *  only enabled when USE_QT_IMAGE is defined.
 */
class GrayImage {
public:
    GrayImage(uint32_t width, uint32_t height, uint8_t fill = 0);
    GrayImage(FT_Bitmap& bitmap);
    GrayImage(const GrayImage& other);
    
    virtual ~GrayImage();

    /*! \brief blit image to this image
     *
     *  \param source source image
     *  \param posX upper left corner of blit position
     *  \param posY upper left corner of blit position
     *
     *  \return false if image did not fit, else true
     */
    bool blit(GrayImage& source, uint32_t posX, uint32_t posY);

    void flipVertically();


    /*! \brief get pointer to given row
     *
     *  This method returns a pointer to the beginning of the given row.
     *  Use it to efficiently read from or write to the image.
     *
     *  \param row the row to return a pointer to
     */
    uint8_t* getRow(uint32_t row) { return (data.data() + pitch*row); }
    uint32_t getWidth() { return width; }
    uint32_t getHeight() { return rows; }
    std::shared_ptr<QImage> getQImage(uint8_t red = 255, uint8_t green = 255, uint8_t blue = 255);


private:
    std::vector<uint8_t> data; //!< the imagedata
    uint32_t pitch; //!< offset between two rows in bytes
    uint32_t width; //!< length of row in bytes
    uint32_t rows; //!< number of rows in image
};

#endif /* GRAYIMAGE_H_ */
