#include "Glyph.h"

#include "Face.h"

#include <cassert>

namespace Ft
{

/**
    Will take a copy of the glyph currently
    loaded by the specified Face object.
**/
Glyph::Glyph( const Face& face )
{
    m_err = FT_Get_Glyph( face.m_face->glyph, &m_glyph );
    assert( m_err == 0 );

    m_metrics = face.m_face->glyph->metrics;
}

/**
    Copy another glyph.
**/
Glyph::Glyph( const Glyph& g )
{
    m_err = FT_Glyph_Copy( g.m_glyph, &m_glyph );
    assert( m_err == 0 );
    m_metrics = g.m_metrics;
}

/**
    Release the glyph.
**/
Glyph::~Glyph()
{
    FT_Done_Glyph( m_glyph );
}

/**
    Copy operator.
**/
const Glyph& Glyph::operator = ( const Glyph& g )
{
    FT_Done_Glyph( m_glyph );
    m_err = FT_Glyph_Copy( g.m_glyph, &m_glyph );
    assert( m_err == 0 );
    
    m_metrics = g.m_metrics;

    return *this;
}

/**
    Return the dimensions of the control box.
**/
FT_BBox Glyph::GetControlBox( FT_UInt mode ) const
{
    FT_BBox bbox;
    FT_Glyph_Get_CBox( m_glyph, mode, &bbox );
    return bbox;
}

/**
    Apply an affine transform to the glyph.

    @param A Transformation matrix.
    @param t Translation vector (coords are 1/64th of a pixel).
**/
void Glyph::Transform( FT_Matrix* A, FT_Vector* t )
{
    FT_Glyph_Transform( m_glyph, A, t );
}

/**
    Convert the font to a bitmap. The bitmap can be retrieved using ExtractBitmap().

    @param mode The render mode.
    @param origin Pointer to offset vector for rendering. If null (default) then (0,0) will be used.
**/
FT_Error Glyph::Render( FT_Render_Mode mode, FT_Vector* origin )
{   
    m_err = FT_Glyph_To_Bitmap( &m_glyph, mode, origin, 1 );
    return m_err;
}

/**
    Copy the glyph's bitmap into the bitmap with the specified width, height, and stride.
    
    @param pDst The buffer into which the bitmap should be extracted.
    @param width Width of the destination bitmap.
    @param height of the destination bitmap.
    @param stride Number of bytes between rows of the destination bitmap.
**/
void Glyph::ExtractBitmap( std::uint8_t* pDst, const int width, const int height, const size_t stride ) const
{
    FT_BitmapGlyph glyph = reinterpret_cast< FT_BitmapGlyph >( m_glyph );
    assert( glyph->bitmap.pixel_mode == FT_PIXEL_MODE_GRAY );

    const int32_t gw = glyph->bitmap.width;
    const int32_t gh = glyph->bitmap.rows;
    const int32_t s = glyph->bitmap.pitch - glyph->bitmap.width + (glyph->bitmap.width-width);
    const int32_t skip = stride-width;

    /// @note Here we need to cap the amount of data we access in the glyph bitmap
    /// becuase width and height could be larger than the glyph's width and height.
    const int32_t copyWidth  = gw > width ? width : gw;
    int32_t copyHeight = gh > height ? height : gh;

    const unsigned char* pSrcRow = glyph->bitmap.buffer;
    const uint8_t* pDstRow = pDst;
    while ( copyHeight-- != 0 )
    {
        uint8_t* pDst = const_cast<uint8_t*>(pDstRow);
        const uint8_t* pSrc = pSrcRow;
        const uint8_t* const pDstEnd = pDst + copyWidth;
        while ( pDst != pDstEnd )
        {
            *pDst++ = *pSrc++;
        }

        pDstRow += stride;
        pSrcRow += glyph->bitmap.pitch;
    }
}

int Glyph::GetBitmapRows() const
{
    FT_BitmapGlyph glyph = reinterpret_cast< FT_BitmapGlyph >( m_glyph );
    return glyph->bitmap.rows;
}

int Glyph::GetBitmapWidth() const
{
    FT_BitmapGlyph glyph = reinterpret_cast< FT_BitmapGlyph >( m_glyph );
    return glyph->bitmap.width;
}

int Glyph::GetBitmapPitch() const
{
    FT_BitmapGlyph glyph = reinterpret_cast< FT_BitmapGlyph >( m_glyph );
    return glyph->bitmap.pitch;
}

int Glyph::GetLeft() const
{
    FT_BitmapGlyph glyph = reinterpret_cast< FT_BitmapGlyph >( m_glyph );
    return glyph->left;        
}

int Glyph::GetTop() const
{
    FT_BitmapGlyph glyph = reinterpret_cast< FT_BitmapGlyph >( m_glyph );
    return glyph->top;
}

} //end namespace Ft

