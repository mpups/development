#include "fast.hpp"

/**
    @param im pointer to the grey-level image data
    @param xsize image width
    @param ysize image height
    @param stride image stride in bytes (i.e. set equal to xsize if rows are not padded).
    @param nonmax the detected corners - this vector will be cleared before it is filled with the results.
*/
void fast9_detect_nonmax(const byte* im, int xsize, int ysize, int stride, int b, std::vector<Corner>& nonmax )
{
    std::vector<int> scores;
    std::vector<Corner> corners;

    fast9_detect(im, xsize, ysize, stride, b, corners );
    fast9_score(im, stride, corners, b, scores );
    nonmax_suppression(corners, scores, nonmax);
}
