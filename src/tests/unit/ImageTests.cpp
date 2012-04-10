    /**
    Copyright (c) mark Pupilli 2012

    Tests in this file relate to image-processing/computer-vision modules.
**/

#include <gtest/gtest.h>

#include "../../vision/Image.h"
#include "../../vision/Pgm.h"
#include "../../vision/ImageProcessing.h"
#include "../../vision/ImageTypes.h"

namespace robo
{

/**
    Test image allocation and row data alignment.
*/
void TestImage()
{
    // Test stride alignment:
    EXPECT_EQ( Image<uint8_t>::DEFAULT_ALIGNMENT_BYTES, Image<uint16_t>::DEFAULT_ALIGNMENT_BYTES );
    EXPECT_EQ( sizeof(float), sizeof(Image<float>::pixel_type) );

    {
        Image<uint8_t> charImg;
        charImg.Allocate(16,11);
        EXPECT_EQ( Image<uint8_t>::DEFAULT_ALIGNMENT_BYTES, charImg.m_stride );
        EXPECT_EQ( 16, charImg.Width() );
        EXPECT_EQ( 11, charImg.Height() );
        EXPECT_EQ( charImg[0], charImg.m_data );
        EXPECT_EQ( charImg[1], charImg[0] + charImg.StrideInBytes() );
    }

    {
        Image<uint16_t> shortImg;
        shortImg.Allocate(8,13);
        EXPECT_EQ( Image<uint16_t>::DEFAULT_ALIGNMENT_BYTES, shortImg.m_stride );
        EXPECT_EQ( shortImg[0], reinterpret_cast<uint16_t*>( shortImg.m_data ) );
        EXPECT_EQ( shortImg[1], shortImg[0] + 8 );
    }

    {
        Image<uint16_t> shortImg2;
        shortImg2.Allocate(9,17);
        EXPECT_EQ( 32, shortImg2.m_stride );

        // test re-allocation:
        shortImg2.Allocate(32,32);
        EXPECT_EQ( 32, shortImg2.Width() );
        EXPECT_EQ( 32, shortImg2.Height() );
    }
}

/**
    Basic test of image 'fill' functions.
*/
void TestImageFill()
{
    Image<uint16_t> shortImg;
    shortImg.Allocate(8,13);
    shortImg.Fill( 42 );
    for( uint32_t j=0; j<1; ++j )
    {
        for ( uint32_t i=0; i<shortImg.Width(); ++i )
        {
            EXPECT_EQ( 42, shortImg[j][i] );
        }
    }
}

/**
    Basic test of SAD matching.
*/
void SadTest()
{
    Image<uint8_t> image;
    image.Allocate( 640, 480 );
    image.Fill( 0 );

    // Fill in an 8 pixel square:
    const AlignedBox region = { {20,20}, 8, 8 };
    image.Fill( region, 0xff );
    EXPECT_EQ( 0x0, image[region.pos.y-1][region.pos.x-1] );
    EXPECT_EQ( 0xff, image[region.pos.y][region.pos.x] );
    EXPECT_EQ( 0xff, image[region.pos.y+region.h-1][region.pos.x+region.w-1] );
    EXPECT_EQ( 0x0, image[region.pos.y+region.h][region.pos.x+region.w] );
    EXPECT_EQ( 0x0, image[image.Height()-1][image.Width()-1] );

    // Write then read back the image:
    bool writeOk = WritePgm( "sad_image.pgm", image );
    ASSERT_TRUE( writeOk );
    bool readOk = ReadPgm( "sad_image.pgm", image );
    ASSERT_EQ( 0x0, image[region.pos.y-1][region.pos.x-1] );
    ASSERT_EQ( 0xff, image[region.pos.y][region.pos.x] );
    ASSERT_EQ( 0xff, image[region.pos.y+region.h-1][region.pos.x+region.w-1] );
    ASSERT_EQ( 0x0, image[region.pos.y+region.h][region.pos.x+region.w] );
    ASSERT_EQ( 0x0, image[image.Height()-1][image.Width()-1] );
    ASSERT_TRUE( readOk );
    WritePgm( "sad_image.pgm", image );

    // Extract a patch description:
    Image<uint8_t> patch;
    patch.CopyFrom( region, image );
    EXPECT_EQ( 0xff, patch[0][0] );
    EXPECT_EQ( 0xff, patch[7][7] );
    WritePgm( "sad_patch.pgm", patch );

    // Now compute the sum-of-absolute differences with the patch perfectly aligned:
    PixelCoord pos = region.pos;
    uint32_t matchError = Sad8x8( image, pos, patch );
    EXPECT_EQ( 0, matchError );

    // And do the same when the patch is out by one column/row:
    pos.x += 1;
    uint32_t colError = Sad8x8( image, pos, patch );
    pos.x -= 1;
    pos.y += 1;
    uint32_t rowError = Sad8x8( image, pos, patch );
    EXPECT_EQ( 8*255, colError );
    EXPECT_EQ( 8*255, rowError );

    // Again, when total misalignment:
    pos.x = 30;
    pos.y = 30;
    uint32_t missError = Sad8x8( image, pos, patch );
    EXPECT_EQ( 8*8*255, missError );

    // Test on a real image:
    {
        GreyImage realImage;
        readOk = ReadPgm( "../../src/tests/unit/data/temple.pgm", realImage );
        EXPECT_TRUE( readOk );
        const AlignedBox region2 = { {191,286}, 8, 8 };
        patch.CopyFrom( region2, realImage );
        pos = region2.pos;
        matchError = Sad8x8( realImage, pos, patch );
        EXPECT_EQ( 0, matchError );

        // test mismatch increases SAD:
        pos.x += 1;
        missError = Sad8x8( realImage, pos, patch );
        EXPECT_GT( missError, matchError );
    }
}

} // end of namespace robo

