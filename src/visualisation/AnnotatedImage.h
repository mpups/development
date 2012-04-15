/*
    Copyright (C) Mark Pupilli 2012, All rights reserved
*/
#ifndef __ANNOTATED_IMAGE_H__
#define __ANNOTATED_IMAGE_H__

#include <glk.h>

#include <vector>

namespace robo
{

class AnnotatedImage : public GLK::ImageWindow
{
public:

    struct Point
    {
        Point( uint16_t x_, uint16_t y_ ) : x(x_), y(y_) {};
        uint16_t x,y;
    };

    struct Line
    {
        Line( const Point& a_, const Point& b_ ) : a(a_), b(b_) {};
        Point a;
        Point b;
    };

    AnnotatedImage( uint16_t w, uint16_t h );
    virtual ~AnnotatedImage();

    void Clear();
    void Add( const Point& p );
    void Add( Line l );

    virtual void Annotate() const;

private:
    std::vector< Point > m_points;
    std::vector< Line > m_lines;
};

} // end of namespace robo

#endif /* __ANNOTATED_IMAGE_H__ */

