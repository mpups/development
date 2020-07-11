#include "Face.h"

#include "Library.h"

#include <assert.h>

namespace Ft
{

/**
    Construct a new font face from a file.

    @param lib Reference to the FreeType library.
    @param font Name of the font file.
    @param index The index of the font face to open (Font files with multiple faces only).
**/
Face::Face( Library& lib, const char* font, FT_Long index )
:
    m_face(0)
{
    m_err = FT_New_Face( lib.m_lib, font, index, &m_face );
    assert( m_err == 0 );
}

/**
    Construct a new font face from a file that has already been loaded into a memory buffer.

    @param lib Reference to the FreeType library.
    @param base Pointer to start of memory buffer.
    @param size Size of memory buffer in bytes.
    @param index The index of the font face to open (Font files with multiple faces only).
**/
Face::Face( Library& lib, const FT_Byte* base, FT_Long size, FT_Long index )
:
    m_face(0)
{
    m_err = FT_New_Memory_Face( lib.m_lib, base, size, index, &m_face );
    assert( m_err == 0 );
}

/**
    Free the font face's resources.

    Calls FT_Done_Face().
**/
Face::~Face()
{
    if ( LoadSuccessful() )
    {
        m_err = FT_Done_Face( m_face );
        assert( m_err == 0 );
    }
}

/**
    Attatch a file to the font face.

    Some font formats hold extra information (e.g. kerning information) in seperate files
    that we need to attatch to the loaded face.
**/
FT_Error Face::AttatchFile( const char* fileName )
{
    m_err = FT_Attach_File( m_face, fileName );
    return m_err;
}

/**
    Request the nominal size in points (calls FT_Request_Size).

    If either dimension is zero it will be assumed equal to the other value.

    If either resolution is zero it will be assumed equal to the other value.

    hres_dpi = max(1,hres_dpi) and vres is the same.

    The default values for hres and vres cause them to be set to 72 dpi.

    Not compatible with FreeType cache API.

    Fixed size fonts will return an error if specified size is not one of the fixed sizes.

    @param width Nominal character width in 26.6 fixed point format.
    @param height Nominal character height in 26.6 fixed point format.
    @param hres Horizontal resolution in dots per inch (DPI).
    @param vres Vertical resolution in dots per inch (DPI).

    @return error code.
**/
FT_Error Face::SetCharSize( FT_F26Dot6 width, FT_F26Dot6 height, FT_UInt hres_dpi, FT_UInt vres_dpi )
{
    return FT_Set_Char_Size( m_face, width, height, hres_dpi, vres_dpi );
}

/**
    Set the font's pixel size directly.

    Fixed size fonts will return an error if specified size is not one of the fixed sizes.

    @param width_px Width in pixels
    @param height_px Height in pixels

    @return error code.
**/
FT_Error Face::SetPixelSizes( FT_UInt width_px, FT_UInt height_px )
{
    return FT_Set_Pixel_Sizes( m_face, width_px, height_px );
}

/**
    Convert a Unicode character code to a glyph index.
    Uses the currently loaded charmap to perform the conversion.

    @param charcode Unicode character.
    @return The glyph index or zero if no corresponding glyph exists.
**/
FT_UInt Face::GetCharIndex( FT_ULong charcode ) const {
    return FT_Get_Char_Index( m_face, charcode );
}

/**
    Load the specified glyph into the 'glyph slot', wher eit can be accessed with ... .

    The glyph might be transformed accoring to SetTransform().

    @param glyphIndex Glyph index (e.g. as returned by GetCharIndex()). For CID fonts this should be the CID key.
    @param flags A combination of  FT_LOAD_... constants (see FreeType2 API).

    @return error code.
**/
FT_Error Face::LoadGlyph( FT_UInt glyphIndex, FT_Int32 flags )
{
    return FT_Load_Glyph( m_face, glyphIndex, flags );
}

/**
    Combines GetCharIndex() and LoadGlyph() in a single call.

    @param charcode Unicode character.
    @param flags A combination of  FT_LOAD_... constants (see FreeType2 API).
    @return error code.
**/
FT_Error Face::LoadChar( FT_ULong charcode, FT_Int32 flags )
{
    return FT_Load_Char( m_face, charcode, flags );
}

/**
    Set the transform that will be applied as glyph images are loaded into the 'glyph slot'.

    @param A Affine transformation matrix.
    @param t Translation vector.
**/
void Face::SetTransform( FT_Matrix* A, FT_Vector* t )
{
    FT_Set_Transform( m_face, A, t );
}

/**
    Create a bitmap for the glyph in the glyph slot.

    @param rendermode Render settings.
    @return error code.
**/
FT_Error Face::Render( FT_Render_Mode renderMode )
{
    return FT_Render_Glyph( m_face->glyph, renderMode );
}

/**
    @param.
    @return error code.
**/
FT_Error Face::SelectCharMap( FT_Encoding encoding )
{
    return FT_Select_Charmap( m_face, encoding );
}

/**
    Return the kerning vector to use when laying out the specified glyphs next to each other.
    Return vector components are in pixels (26.6) for scalable fonts, and font units for non-scalable fonts.
**/
FT_Vector Face::GetKerning( FT_UInt leftGlyphIndex, FT_UInt rightGlyphIndex, FT_UInt mode ) const
{
    FT_Vector kerning;
    m_err = FT_Get_Kerning( m_face, leftGlyphIndex, rightGlyphIndex, mode, &kerning );
    return kerning;
}

FT_Vector Face::GetKerning( const char c, const char nextc, FT_UInt mode ) const
{
    return GetKerning( GetCharIndex(c), GetCharIndex(nextc), mode );
}

} // end namespace Ft
