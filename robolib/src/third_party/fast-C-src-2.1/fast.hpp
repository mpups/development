#ifndef __FAST_CPP_HPP__
#define __FAST_CPP_HPP__

#include <vector>

struct Corner
{
    Corner( int x_, int y_ ) : x(x_), y(y_) {};
    int x,y;
};

typedef unsigned char byte;

void fast9_detect(const byte* im, int xsize, int ysize, int stride, int b, std::vector<Corner>& corners );

void fast9_score(const byte* i, int stride, const std::vector<Corner>& corners, int b, std::vector<int>& scores );

void nonmax_suppression(const std::vector<Corner>& corners, const std::vector<int>& scores, std::vector<Corner>& nonmax );

void fast9_detect_nonmax(const byte* im, int xsize, int ysize, int stride, int b, std::vector<Corner>& corners );

#endif /* __FAST_CPP_HPP__ */

