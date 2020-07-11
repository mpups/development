#ifndef FREETYPE_CACHE_H
#define FREETYPE_CACHE_H

#include "Library.h"
#include "Face.h"
#include "Glyph.h"

#include <cstdint>
#include <vector>
#include <memory>
#include <functional>

namespace Ft
{

class Cache
{
public:
    Cache( const Cache& ) = delete;
    Cache( Ft::Library& lib, const char* font, FT_Long index = 0 );
    virtual ~Cache();

    void SetSizePx( unsigned int size );
    int32_t GetSizePx() const { return m_sizePx; };
    void SetSizePt( float size, unsigned int dpi );
    float GetLineHeightPx() const { return m_fontFace.GetSize()->metrics.height / METRIC_BASE; };
    const Ft::Face& GetFace() const { return m_fontFace; };

    bool FontLoaded() const { return m_fontFace.LoadSuccessful(); };
    void FillCache();

    const Ft::Glyph* GetGlyph( const size_t index ) const;
    void ClearCache();
    void CacheGlyph( const size_t i );

    typedef std::function< void(const unsigned char, Ft::Glyph&)> CacheCallBack;
    void SetCacheCallBack( CacheCallBack&& callback );

    const Ft::Glyph& CacheAndGetGlyph( const size_t c );

    static constexpr float  METRIC_BASE  = 64.f;
    static constexpr float  ADVANCE_BASE = 65536.f;

    typedef std::vector< std::unique_ptr<Ft::Glyph> > GlyphStorage;
    GlyphStorage::const_iterator begin() const { return m_cache.cbegin(); }
    GlyphStorage::const_iterator end()   const { return m_cache.cend(); }

private:
    static constexpr size_t NUM_GLYPHS   = 256;

    Ft::Library& m_freeType;
    Ft::Face     m_fontFace;
    GlyphStorage m_cache;

    int32_t m_sizePx;

    CacheCallBack m_callAfterCaching;
};

} // end of namesapce Ft

#endif // FREETYPE_CACHE_H
