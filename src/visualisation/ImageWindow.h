#ifndef IMAGE_WINDOW_H
#define IMAGE_WINDOW_H

#ifndef ARM_BUILD

#include <glk.h>

/**
    Simple class to display images.

    The creates a window which runs in a separate thread to which images can be posted for display.

    Currently only supports grey-scale images.
**/
class ImageWindow : public GLK::GlThreadWindow
{
public:

    /**
        Enums control (if) and how the image is scaled to fit the window.
    **/
    enum DisplayMode
    {
        FixedSize,        //< No scaling.
        FixedAspectRatio, //< Image is scaled but aspect ratio is preserved.
        ScaleToFit       //< Image is scaled and stretched to fit the window exactly.
    };

    ImageWindow();
    ~ImageWindow();
    
    virtual bool InitGL();
    virtual void DestroyGL();
    virtual void Resize(const unsigned int, const unsigned int);
    virtual bool Update( unsigned int );
    virtual void Render();
    virtual void TimerExpired( int id );

    void PostImage( uint8_t* imageBuffer, uint16_t width_px, uint16_t height_px, DisplayMode mode = FixedSize );

protected:
    /**
        Structure used to send image data to window.
    **/
    struct ImageData
    {
        uint8_t* ptr;
        uint16_t w;
        uint16_t h;
        DisplayMode mode;
    };

private:
    GLK::MessageQueue<ImageData> m_msgs; // Message queue for receiving images.
    GLuint m_lumTex;    // Texture Id.
    ImageData   m_image; // The current image data being displayed.
};

#endif

#endif // IMAGE_WINDOW_H

