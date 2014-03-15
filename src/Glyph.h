#ifndef FreeType_GLYPH_H
#define FreeType_GLYPH_H

extern "C"
{
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
}

#include "ft_exports.h"
#include <cstdint>

namespace Ft
{

// fwd decl
class Face;

/**
    Class for manipulating extracted FreeType glyphs.
**/
class FTCPP_API Glyph
{
public:
    Glyph() = delete;
    Glyph( const Face& face );   
    Glyph( const Glyph& glyph );   
    virtual ~Glyph();

    const Glyph& operator = ( const Glyph& glyph );

    //! Return the error code for the last operation.
    FT_Error GetError() const { return m_err; };

    //! Return true if the font is a bitmap (i.e. not from a scalable font face).
    bool IsBitmapFont() const { return m_glyph->format == FT_GLYPH_FORMAT_BITMAP; };

    FT_BBox GetControlBox( FT_UInt mode = FT_GLYPH_BBOX_PIXELS ) const;

    void Transform( FT_Matrix* A, FT_Vector* t );

    //! Return the (transformed) advance vector.
    FT_Vector GetAdvance() const { return m_glyph->advance; };

    FT_Error Render( FT_Render_Mode mode = FT_RENDER_MODE_NORMAL, FT_Vector* origin = nullptr );

    void ExtractBitmap(std::uint8_t *data, const int width, const int height, const size_t stride ) const;
    
    int GetBitmapRows() const;
    int GetBitmapWidth() const;
    int GetBitmapPitch() const;
    int GetLeft() const;
    int GetTop() const;

    //! Returns the original glyph metrics: if the glyph has been transformed then these are invalid.
    const FT_Glyph_Metrics& GetMetrics() const { return m_metrics; };

protected:

private:
    FT_Glyph m_glyph;
    FT_Error m_err;
    FT_Glyph_Metrics m_metrics;
};

} //end namespace Ft

#endif // FreeType_GLYPH_H
