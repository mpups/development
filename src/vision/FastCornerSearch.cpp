#include "FastCornerSearch.h"

#include "ImageGeometry.h"

namespace robo
{

/**
    Build an index into the fast corner result set.

    @corners A set of fast corner results for which the search will operate.
*/
FastCornerSearch::FastCornerSearch( const std::vector<PixelCoord>& corners )
:
    m_corners ( corners )
{
    // Record index where each row starts:
    int yMax = corners[ corners.size()-1 ].y;
    m_rowIndex.resize( yMax + 1, -1 );
    int prevRow = -1;
    for ( unsigned int i=0; i<corners.size(); ++i )
    {
        if ( corners[i].y != prevRow )
        {
            m_rowIndex[corners[i].y] = i;
            prevRow = corners[i].y;
        }
    }
}

FastCornerSearch::~FastCornerSearch()
{
}

int FastCornerSearch::GetRow( int row ) const
{
    return m_rowIndex[row];
}

/**
    Get a set of indices which identify corners within the specified bounding rectangle.
    @param results a vector to which the resulting indices will be appended.
*/
void FastCornerSearch::GetRectIndices( const AlignedBox& rect, std::vector<int>& results ) const
{
    for ( int r = rect.pos.y; r<rect.pos.y+rect.h; ++r )
    {
        ScanRow( r, rect.pos.x, rect.pos.x+rect.w, results );
    }
}

void FastCornerSearch::ScanRow( int row, int x1, int x2, std::vector<int>& results ) const
{
    int index = m_rowIndex[row];
    if (index == -1 )
    {
        return;
    }

    // Scan along the row until we are at first corner inside the rectangle:
    while ( index < m_corners.size() && m_corners[index].y == row && m_corners[index].x < x1 )
    {
        index += 1;
    }

    while ( index < m_corners.size() && m_corners[index].y == row && m_corners[index].x < x2 )
    {
        results.push_back( index );
        index += 1;
    }
}

} // end namespace robo

