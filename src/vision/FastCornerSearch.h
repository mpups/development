/*
    Copyright (C) Mark Pupilli 2012, All rights reserved
*/
#ifndef __FAST_CORNER_SEARCH_H__
#define __FAST_CORNER_SEARCH_H__

#include <stdint.h>

#include <vector>

namespace robo
{

class PixelCoord;
class AlignedBox;

/**
    Object allows efficient searching of rectangular regions
    in a set of Fast corner results (as returned from FastCornerThread::RetrieveResults()).
*/
class FastCornerSearch
{
public:
    FastCornerSearch( const std::vector<PixelCoord>& corners );
    virtual ~FastCornerSearch();

    int GetRow( int row ) const;
    void GetRectIndices( const AlignedBox& rect, std::vector<int>& results ) const;

private:
    void ScanRow( int row, int y1, int y2, std::vector<int>& results ) const;

    std::vector<int> m_rowIndex;
    const std::vector<PixelCoord>& m_corners;
};

} // end namespace robo

#endif /* __FAST_CORNER_SEARCH_H__ */

