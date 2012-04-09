    /**
    Copyright (c) mark Pupilli 2012

    Tests in this file relate to image-processing/computer-vision modules.
**/

#include <gtest/gtest.h>

#include "../../vision/Image.h"
#include "../../vision/Pgm.h"

namespace robo
{

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
    }
}

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

void SadTest()
{
    Image<uint8_t> image;
    image.Allocate( 640, 480 );
    image.Fill( 0 );

    // Fill in a 10 pixel square:
    AlignedBox region = { {20,20}, 10, 10 };
    image.Fill( region, 0xff );
    EXPECT_EQ( 0x0, image[region.pos.y-1][region.pos.x-1] );
    EXPECT_EQ( 0xff, image[region.pos.y][region.pos.x] );
    EXPECT_EQ( 0xff, image[region.pos.y+region.h-1][region.pos.x+region.w-1] );
    EXPECT_EQ( 0x0, image[region.pos.y+region.h][region.pos.x+region.w] );
    EXPECT_EQ( 0x0, image[image.Height()-1][image.Width()-1] );

    WritePgm( "sad_image.pgm", image );

    // Extract a patch description:
    Image<uint8_t> patch;
    region.pos.x -= 4;
    region.pos.y -= 4;
    region.w = 8;
    region.h = 8;
    patch.CopyFrom( region, image );
    EXPECT_EQ( 0, patch[3][3] );
    EXPECT_EQ( 0xff, patch[4][4] );

    WritePgm( "sad_patch.pgm", patch );
}

} // end of namespace robo

