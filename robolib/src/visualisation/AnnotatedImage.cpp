#include "AnnotatedImage.h"

namespace robo
{

AnnotatedImage::AnnotatedImage( uint16_t w, uint16_t h )
:
    ImageWindow( w, h )
{
}

AnnotatedImage::~AnnotatedImage()
{
}

/**
    Clear all the annotations.
*/
void AnnotatedImage::Clear()
{
    m_points.clear();
    m_lines.clear();
};

/**
    Add a point annotation.
*/
void AnnotatedImage::Add( const Point& p )
{
    m_points.push_back( p );
};

/**
    Add a line annotation.
*/
void AnnotatedImage::Add( Line l )
{
    m_lines.push_back( l );
}

/**
    Render the points and lines that have been added to the annotation since the last call to Clear().
*/
void AnnotatedImage::Annotate() const
{
    glMatrixMode( GL_PROJECTION );
    glPushMatrix();
    glLoadIdentity();
    glOrtho( 0, GetImageData().w, GetImageData().h, 0, -1.f, 1.f );

    glColor3f( 1, 0, 0 );
    glBegin( GL_LINES );
    for ( unsigned int i=0; i<m_lines.size(); ++i )
    {
        const Line& l = m_lines[i];
        glVertex2f( l.a.x, l.a.y );
        glVertex2f( l.b.x, l.b.y );
    }
    glEnd();

    glColor3f( 0, 1, 0 );
    glPointSize( GetWidth()/640.f );
    glBegin( GL_POINTS );
    for ( unsigned int i=0; i<m_points.size(); ++i )
    {
        const Point& p = m_points[i];
        glVertex2f( p.x, p.y );
    }
    glEnd();

    glMatrixMode( GL_PROJECTION );
    glPopMatrix();
}

} // end of namespace robo

