#ifndef IMAGE_WINDOW_H
#define IMAGE_WINDOW_H

#ifndef ARM_BUILD

#include <glk.h>

class ImageWindow : public GLK::GlThreadWindow
{
public:
    ImageWindow();
    ~ImageWindow();
    
    virtual bool InitGL();
    virtual void DestroyGL();
    virtual void Resize(const unsigned int, const unsigned int);
    virtual bool Update( unsigned int );
    virtual void Render();
    virtual void TimerExpired( int id );

    void PostImage( uint8_t* imageBuffer );

protected:

private:
    GLK::MessageQueue<uint8_t*> m_msgs;
    GLuint m_lumTex;
};

#endif

#endif // IMAGE_WINDOW_H

