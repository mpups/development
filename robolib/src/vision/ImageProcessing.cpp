
#include "ImageProcessing.h"

namespace robo
{

/**
    Compute the sum of absolute difference (SAD) between the patch and the image
    at the specified position. No bounds checking or image dimension checking
    are performed by this function.

    @param image arbitrary image in which we want to atch th patch.
    @param pos position of top left pixel in the image where SAD will be computed.
    @param patch an 8x8 image patch (or at least only the upper left 8x8 block will be used in the computation).
*/
uint32_t Sad8x8( const Image<uint8_t>& image, const PixelCoord& pos, const Image<uint8_t>& patch )
{
    const uint8_t* imgRowPtr = image.PixelPtr( pos );
    const uint8_t* patchRowPtr = patch[0];
    uint32_t sad = 0;
    for ( uint32_t j=8; j!=0; --j )
    {
        const uint8_t* imgPixelPtr = imgRowPtr;
        const uint8_t* patchPixelPtr = patchRowPtr;
        for ( uint32_t i=8; i!=0; --i )
        {
            uint32_t diff = *imgPixelPtr - *patchPixelPtr;
            sad += abs(diff);
            ++imgPixelPtr;
            ++patchPixelPtr;
        }

        imgRowPtr += image.StrideInBytes();
        patchRowPtr += patch.StrideInBytes();
    }

    return sad;
}

} // end of namespace robo

