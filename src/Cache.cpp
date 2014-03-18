#include "Cache.h"

#include <cmath>
#include <cassert>

namespace Ft
{

static constexpr size_t DEFAULT_FONT_SIZE = 16;

Cache::Cache( Ft::Library& lib, const char* font, FT_Long index )
:
    m_freeType ( lib ),
    m_fontFace ( m_freeType, font, index ),
    m_cache    ( NUM_GLYPHS ),
    m_sizePx   ( DEFAULT_FONT_SIZE )
{
    m_fontFace.SetPixelSizes( m_sizePx, m_sizePx );
}

Cache::~Cache()
{
}

void Cache::ClearCache()
{
    for ( auto& glyphPtr : m_cache )
    {
        glyphPtr.reset();
    }
}

void Cache::FillCache()
{
    for ( size_t c=0; c<m_cache.size(); ++c )
    {
        CacheGlyph(c);
    }
}

void Cache::CacheGlyph( const size_t i )
{
    m_fontFace.LoadChar( (unsigned char)i );
    m_cache[i].reset( new Ft::Glyph( m_fontFace ) );
    m_cache[i]->Render();
    assert( m_cache[i] != nullptr );
    if (m_callAfterCaching)
    {
        m_callAfterCaching(i,*m_cache[i]);
    }
}

const Ft::Glyph* Cache::GetGlyph( const size_t index ) const
{
    return m_cache[index].get();
}

const Ft::Glyph& Cache::CacheAndGetGlyph( const size_t c )
{
    if ( GetGlyph(c) == nullptr )
    {
        CacheGlyph(c);
    }
    const Ft::Glyph* p = GetGlyph(c);
    assert( p != nullptr );
    return *p;
}

/**
    Set a function that will be called everytime a glyph
    is internally cached.
*/
void Cache::SetCacheCallBack( CacheCallBack&& callback )
{
    m_callAfterCaching = callback;
}

void Cache::SetSizePx( unsigned int size )
{
    ClearCache();
    m_sizePx = size;
    m_fontFace.SetPixelSizes( m_sizePx, m_sizePx );
}

void Cache::SetSizePt( float size, unsigned int dpi )
{
    ClearCache();

    // convert float to 26.6 fixed point:
    int fixed = static_cast<int>( floor(size) );
    float pc = 63.f*(size - fixed);
    int frac = static_cast<int>( pc );
    FT_F26Dot6 fixedSize = fixed << 6;
    fixedSize += frac & 0x3F;

    // set the face size and res:
    m_fontFace.SetCharSize( fixedSize, fixedSize, dpi, dpi );
    m_sizePx = m_fontFace.GetSize()->metrics.x_ppem;
    assert( m_fontFace.GetSize()->metrics.x_ppem == m_fontFace.GetSize()->metrics.y_ppem );
}

} // end of namesapce Ft
