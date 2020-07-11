#ifndef FreeType_FACE_H
#define FreeType_FACE_H

extern "C"
{
#include <ft2build.h>
#include FT_FREETYPE_H
}

#include "ft_exports.h"

namespace Ft
{

//fwd decl
class Library;

/**
    Wrapper for a handle to FreeType2 FT_Face handle.
**/
class FTCPP_API Face
{
    friend class Glyph;

public:
    Face() = delete;
    Face( Library& lib, const char* font, FT_Long index = 0 );
    Face( Library& lib, const FT_Byte* base, FT_Long size, FT_Long index = 0 );
    ~Face();

    bool LoadSuccessful() const { return m_face != 0; };
    FT_Error GetError() const { return m_err; };

    FT_Error AttatchFile( const char* fileName );

    //! Return the family of the loaded font.
    FT_String* GetFamilyName() const { return m_face->family_name; };
    
    //! Return the style of the loaded font.
    FT_String* GetStyleName()  const { return m_face->style_name; };

    //! Return true if the font face is scalable to any size.
    bool IsScalable() const { return FT_IS_SCALABLE( m_face ) != 0; };

    //! Return true if the font contains kerning information.
    bool HasKerning() const { return FT_HAS_KERNING( m_face ) != 0; };

    //! Return true if this is a monospaced font (e.g. courier, consolas etc).
    bool IsFixedWidth() const { return (m_face->face_flags & FT_FACE_FLAG_FIXED_WIDTH) != 0; };

    // Query font face:
    FT_Long GetNumGlyphs() const { return m_face->num_glyphs; };
    FT_Long GetFlags() const { return m_face->face_flags; };
    FT_Size GetSize() const { return m_face->size; };
    float GetMaxAdvance() const { return m_face->size->metrics.max_advance / 64.f; };
    FT_Short GetScaledAscender() const { return m_face->size->metrics.ascender; };
    FT_Short GetScaledDescender() const { return m_face->size->metrics.descender; };

    // Set/modify font face:
    FT_Error SetCharSize( FT_F26Dot6 width, FT_F26Dot6 height, FT_UInt hres_dpi = 0, FT_UInt vres_dpi = 0 );
    FT_Error SetPixelSizes( FT_UInt width_px , FT_UInt height_px );

    // Glyph handling:
    FT_UInt GetCharIndex( FT_ULong charcode ) const;
    FT_Error LoadGlyph( FT_UInt glyphIndex, FT_Int32 flags );
    FT_Error LoadChar( FT_ULong charcode, FT_Int32 flags = FT_LOAD_DEFAULT );
    void SetTransform( FT_Matrix* A, FT_Vector* t );
    FT_Error Render( FT_Render_Mode renderMode = FT_RENDER_MODE_NORMAL );
    FT_Error SelectCharMap( FT_Encoding encoding );
    FT_GlyphSlot GetGlyphSlot() const { return m_face->glyph; };
    
    FT_Vector GetKerning( FT_UInt leftGlyphIndex, FT_UInt rightGlyphIndex, FT_UInt mode = FT_KERNING_DEFAULT ) const;
    FT_Vector GetKerning( const char c, const char nextc, FT_UInt mode ) const;

protected:

private:
    FT_Face m_face;
    mutable FT_Error m_err;
};

} // end namespace Ft

#endif // FreeType_FACE_H
