#include "fast.hpp"

#include <assert.h>

inline bool Compare( int X, int Y )
{
    return X >= Y;
}

/**
    @param corners the input set of corners.
    @param scores correponding to the input set of corners.
    @param nonmax output set of corners after non-maximal supression. This vector will be cleared before it is filled with the results.
*/
void nonmax_suppression(const std::vector<Corner>& corners, const std::vector<int>& scores, std::vector<Corner>& nonmax )
{
    nonmax.clear();
    const int sz = static_cast<int>( corners.size() ); 

    /*Point above points (roughly) to the pixel above the one of interest, if there
    is a feature there.*/
    int point_above = 0;
    int point_below = 0;

    if(corners.size() < 1)
    {
        return;
    }

	/* Find where each row begins
	   (the corners are output in raster scan order). A beginning of -1 signifies
	   that there are no corners on that row. */
	int last_row = corners[ corners.size()-1 ].y;
    std::vector<int> row_start;
    row_start.resize( last_row + 1, -1 );
    int prev_row = -1;
    for( unsigned int i=0; i<corners.size(); ++i )
    {
        if(corners[i].y != prev_row)
        {
            row_start[corners[i].y] = i;
            prev_row = corners[i].y;
        }
    }

    assert( scores.size() == corners.size() );
	for( unsigned int i=0; i<corners.size(); ++i )
	{
		int score = scores[i];
		Corner pos = corners[i];
			
		/*Check left */
		if(i > 0)
			if(corners[i-1].x == pos.x-1 && corners[i-1].y == pos.y && Compare(scores[i-1], score))
				continue;
			
		/*Check right*/
		if(i < (sz - 1))
			if(corners[i+1].x == pos.x+1 && corners[i+1].y == pos.y && Compare(scores[i+1], score))
				continue;
			
		/*Check above (if there is a valid row above)*/
		if(pos.y != 0 && row_start[pos.y - 1] != -1) 
		{
			/*Make sure that current point_above is one
			  row above.*/
			if(corners[point_above].y < pos.y - 1)
				point_above = row_start[pos.y-1];
			
			/*Make point_above point to the first of the pixels above the current point,
			  if it exists.*/
			for(; corners[point_above].y < pos.y && corners[point_above].x < pos.x - 1; point_above++)
			{}
			
			
			for(int j=point_above; corners[j].y < pos.y && corners[j].x <= pos.x + 1; j++)
			{
				int x = corners[j].x;
				if( (x == pos.x - 1 || x ==pos.x || x == pos.x+1) && Compare(scores[j], score))
					goto cont;
			}
			
		}
			
		/*Check below (if there is anything below)*/
		if(pos.y != last_row && row_start[pos.y + 1] != -1 && point_below < sz) /*Nothing below*/
		{
			if(corners[point_below].y < pos.y + 1)
				point_below = row_start[pos.y+1];
			
			/* Make point below point to one of the pixels belowthe current point, if it
			   exists.*/
			for(; point_below < sz && corners[point_below].y == pos.y+1 && corners[point_below].x < pos.x - 1; point_below++)
			{}

			for(int j=point_below; j < sz && corners[j].y == pos.y+1 && corners[j].x <= pos.x + 1; j++)
			{
				int x = corners[j].x;
				if( (x == pos.x - 1 || x ==pos.x || x == pos.x+1) && Compare(scores[j],score))
					goto cont;
			}
		}
		
		nonmax.push_back( corners[i] );
		cont:
			;
	}

}

