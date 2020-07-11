#include "Measure.h"
#include "Cache.h"

#include <cassert>

namespace Ft
{

Measure::Measure( Cache& cache )
:
    m_cache(cache)
{
}

/**
    Compute and return the absolute offset to the start of each character.

    Newlines cause a single spaced vertical offset of line-height.
*/
Measure::Measurements Measure::Offset( const std::string& text, bool useKerning )
{
    const Face& fontFace = m_cache.GetFace();
    assert( fontFace.GetError() == 0 );
    assert( text.size() > 0 );

    Vector offset{ 0.f, 0.f };
    Measurements measurements;
    measurements.reserve(text.size());
    measurements.push_back(offset);

    // Set the face size and get required metrics
    useKerning = fontFace.HasKerning() && useKerning;

    std::string::const_iterator itr = text.cbegin();
    const std::string::const_iterator endItr = text.end()-1;
    char nextc = *itr;
    char c;
    while ( itr != endItr )
    {
        c = nextc;
        nextc = *(++itr);

        if ( c=='\n' )
        {
            offset.x = 0.f;
            offset.y -= m_cache.GetLineHeightPx();
        }
        else
        {
            const Ft::Glyph& glyph = m_cache.CacheAndGetGlyph( static_cast<size_t>(c) );
            const FT_Vector advance = glyph.GetAdvance();
            offset.x += advance.x/Cache::ADVANCE_BASE;
            offset.y += advance.y/Cache::ADVANCE_BASE;

            // Kerning:
            if ( useKerning )
            {
                const FT_Vector kerning = fontFace.GetKerning( c, nextc );
                offset.x += kerning.x/Cache::METRIC_BASE;
                offset.y += kerning.y/Cache::METRIC_BASE;
            }
        }

        measurements.push_back(offset);
    }

    assert( measurements.size() == text.size() );
    return measurements;
}

/**
    Compute and return all the translation deltas that come after each character.

    @note Newlines cause a delta that corresponds to a single spaced vertical offset of line-height
    AND a horizontal offset that returns the 'cursor' to the start of the line.
*/
Measure::Measurements Measure::Delta( const std::string& text, bool useKerning )
{
    const Face& fontFace = m_cache.GetFace();
    assert( fontFace.GetError() == 0 );
    assert( text.size() > 0 );

    Measurements deltas;
    deltas.reserve(text.size());

    // Set the face size and get required metrics
    useKerning = fontFace.HasKerning() && useKerning;

    std::string::const_iterator itr = text.cbegin();
    const std::string::const_iterator endItr = text.end();
    char nextc = *itr;
    char c;
    float lineLength = 0.f;
    while ( itr != endItr )
    {
        c = nextc;
        nextc = *(++itr);

        if ( c=='\n' )
        {
            deltas.push_back({-lineLength,-m_cache.GetLineHeightPx()});
            lineLength = 0.f;
        }
        else
        {
            const Ft::Glyph& glyph = m_cache.CacheAndGetGlyph( static_cast<size_t>(c) );
            const FT_Vector advance = glyph.GetAdvance();
            Vector offset
            {
                advance.x/Cache::ADVANCE_BASE,
                advance.y/Cache::ADVANCE_BASE
            };

            // Kerning:
            if ( useKerning )
            {
                const FT_Vector kerning = fontFace.GetKerning( c, nextc );
                offset.x += kerning.x/Cache::METRIC_BASE;
                offset.y += kerning.y/Cache::METRIC_BASE;
            }

            deltas.push_back(offset);
            lineLength += offset.x;
        }
    }

    assert( deltas.size() == text.size() );
    return deltas;
}

/**
    Return the offset that the character at index 'i' would be located if the string were rendered at the currently cached pixel size.

    @param text The text we want to measure.
    @param i Index to measure up to. Must be within the bounds of the string.
    @param useKerning If true (and the font face supports it) then adjust measurements for kerning.
**/
Measure::Vector Measure::GetOffsetPx( const std::string& text, const std::size_t i, bool useKerning )
{
    return Offset( text, useKerning )[i];
}

/**
    This only works for well authored fonts (where the ascender and descender are meaningful).
*/
Measure::BBoxi Measure::ComputeBoundingBox(const std::string& text, const bool kerned, Ft::Cache& cache)
{
    const auto textOffsets = Ft::Measure(cache).Offset(text,kerned);
    const float textWidth   = textOffsets.back().x;
    const float textAscent  = (cache.GetFace().GetScaledAscender()/Ft::Cache::METRIC_BASE);
    const float textDescent = (cache.GetFace().GetScaledDescender()/Ft::Cache::METRIC_BASE);

    const int firstLeftBearing = cache.CacheAndGetGlyph(text.front()).GetLeft();
    const float lastAdvance = cache.CacheAndGetGlyph(text.back()).GetLeft() + cache.CacheAndGetGlyph(text.back()).GetMetrics().width/Ft::Cache::METRIC_BASE;

    return BBoxi
    {
        firstLeftBearing,
        static_cast<std::int32_t>(-textAscent),
        static_cast<std::int32_t>(textWidth+lastAdvance),
        static_cast<std::int32_t>(-textDescent)
    };
}

} // end namespace Ft
