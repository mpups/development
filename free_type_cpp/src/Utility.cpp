#include "Utility.h"
#include "Cache.h"
#include "Measure.h"

namespace Ft
{

std::vector<std::uint8_t> CreateTableauImage(const Cache& cache)
{
    const std::int32_t glyphSizePx = cache.GetSizePx();
    const std::int32_t size = glyphSizePx*16;
    std::vector<uint8_t> image(size*size,0);

    std::uint32_t index = 0;
    for ( const auto& glyphPtr : cache )
    {
        if ( glyphPtr != nullptr )
        {
            std::uint32_t c = index % 16;
            std::uint32_t r = index / 16;
            glyphPtr->ExtractBitmap(image.data()+(size*glyphSizePx*r)+(glyphSizePx*c),glyphSizePx,glyphSizePx,size);
        }
        index += 1;
    }

    return image;
}

/**
    Most basic rendering of font into an image.

    @note NO BOUNDS CHECKING IS PERFORMED!

    @param x Horizontal pixel coord where text begins.
    @param y Vertical pixel coord (from top of image) of baseline (upon which text rests).
*/
void RenderString( Cache& cache,
                   const std::string& text,
                   std::uint8_t* image,
                   const int32_t width, const int32_t height, const int32_t stride,
                   const int32_t x, const int32_t y
                 )
{
    auto advances = Measure(cache).Delta(text,true);

    int32_t penX = x;
    int32_t penY = y;
    size_t i = 0;
    for ( const char c : text )
    {
        const Ft::Glyph& glyph = cache.CacheAndGetGlyph(c);
        const size_t offset = (penY-glyph.GetTop())*stride + (penX + glyph.GetLeft());
        RenderGlyph( glyph, image + offset, width, height, stride );

        const Measure::Vector advance = advances[i];
        penX += advance.x;
        penY += advance.y;

        i += 1;
    }
}

void RenderGlyph( const Ft::Glyph& glyph,
                  std::uint8_t* image,
                  const int32_t width, const int32_t height, const int32_t stride)
{
    glyph.ExtractBitmap(image,width,height,stride);
}

void RenderBoundingBox( uint8_t* const image, const uint32_t stride,
                        const Measure::BBoxi bbox )
{
    uint8_t* pStart = image + (bbox.top*stride) + bbox.left;
    for ( int32_t r=bbox.top; r<=bbox.bottom; ++r )
    {
        uint8_t* pRow = pStart;
        for ( int32_t c=bbox.left; c<=bbox.right; ++c )
        {
            *pRow++ = 0;
        }

        pStart += stride;
    }
}

} // end namespace Ft
