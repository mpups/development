#ifndef FT_UTILITY_H
#define FT_UTILITY_H

#include <vector>
#include <cstdint>
#include <string>

#include "Measure.h"

namespace Ft
{

class Cache;
class Glyph;

std::vector<std::uint8_t> CreateTableauImage(const Cache& cache);

void RenderString(Cache &cache,
                   const std::string &text,
                   std::uint8_t *image,
                   const int32_t width, const int32_t height, const int32_t stride,
                   const int32_t x, const int32_t y
                 );

void RenderGlyph( const Glyph& glyph,
                  std::uint8_t* image,
                  const int32_t width, const int32_t height, const int32_t stride);

void RenderBoundingBox( uint8_t* const image, const uint32_t stride,
                        const Measure::BBoxi bbox );

} // end namespace Ft

#endif // FT_UTILITY_H
