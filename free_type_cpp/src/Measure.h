#ifndef FT_MEASURE_H
#define FT_MEASURE_H

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace Ft
{

class Cache;

std::vector<uint8_t> CreateTableauImage(const Cache& cache);

class Measure
{
public:

    struct BBoxi
    {
        std::int32_t left,top,right,bottom;

        const BBoxi& operator + (const BBoxi& bb)
        {
            left += bb.left;
            top  += bb.top;
            right += bb.right;
            bottom += bb.bottom;
            return *this;
        }
    };

    Measure( Cache& cache );
    virtual ~Measure() {};

    struct Vector
    {
        float x,y;
    };
    typedef std::vector<Vector> Measurements;

    Vector GetOffsetPx(const std::string& text, const std::size_t i, bool useKerning );

    Measurements Offset(const std::string& text, bool useKerning );
    Measurements Delta( const std::string& text, bool useKerning );

    static BBoxi ComputeBoundingBox(const std::string& text, const bool kerned, Ft::Cache& cache);


private:
    Cache& m_cache;
};

} // end of namespace Ft

#endif // FT_MEASURE_H
