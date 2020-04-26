#include "UnicapCapture.h"

#include <assert.h>

#include "video_conversion.h"
#include <memory.h>

#include <functional>

UnicapCapture::UnicapCapture()
:
    m_buffer        ( nullptr ),
    m_frameCount    ( 0 ),
    m_retrievedCount( 0 ),
    m_time          ({0,0})
{
    if ( m_camera.IsOpen() )
    {
        pthread_mutex_init( &m_mutex, 0 );
        pthread_cond_init( &m_cond, 0 );

        const std::size_t bufferSize = m_camera.GetFormatBufferSize();
        int err = posix_memalign( (void**)&m_buffer, 16, bufferSize );
        assert(err == 0);
        m_camera.SetCaptureCallback( std::bind( &UnicapCapture::OnCapture, std::ref(*this), std::placeholders::_1, std::placeholders::_2 ) );
    }
}

UnicapCapture::~UnicapCapture()
{
    free( m_buffer );

    pthread_cond_destroy( &m_cond );
    pthread_mutex_destroy( &m_mutex );
}

/**
    This gets called in the context of unicap's capture thread when a new frame arrives.
    It then makes a copy of the capture buffer and signals a semaphore that an image is ready.
*/
void UnicapCapture::OnCapture( uint8_t* buffer, const timespec& time )
{
    pthread_mutex_lock( &m_mutex );

    memcpy( m_buffer, buffer, m_camera.GetFormatBufferSize() );

    m_time = time;
    m_frameCount += 1;

    pthread_cond_signal( &m_cond );
    pthread_mutex_unlock( &m_mutex );
}

bool UnicapCapture::IsOpen() const
{
    return m_camera.IsOpen();
}

/**
    Registers the callback routine and starts capturing frames.
*/
void UnicapCapture::StartCapture()
{
    m_camera.StartCapture();
}

/**

*/
void UnicapCapture::StopCapture()
{
    m_camera.StopCapture();
}

/**
    Blocks until a frame is captured.

    You must call DoneFrame() after each call to GetFrame().

    @return true if capture was successful.
**/
bool UnicapCapture::GetFrame()
{
    pthread_mutex_lock( &m_mutex );
    while ( m_retrievedCount >= m_frameCount )
    {
        pthread_cond_wait( &m_cond, &m_mutex );
    }

    m_retrievedCount = m_frameCount;

    return true;
}

/**
    This releases the frame so that the driver's capture thread can return a new frame.
**/
void UnicapCapture::DoneFrame()
{
    pthread_mutex_unlock( &m_mutex );
}

int32_t UnicapCapture::GetFrameWidth() const
{
    return m_camera.GetFrameWidth();
}

int32_t UnicapCapture::GetFrameHeight() const
{
    return m_camera.GetFrameHeight();
}

timespec UnicapCapture::GetFrameTimestamp() const
{
    return m_time;
}

uint64_t UnicapCapture::GetGuid() const
{
    return m_camera.GetGuid();
}

const char* UnicapCapture::GetVendor() const
{
    return m_camera.GetVendor();
}

const char* UnicapCapture::GetModel() const
{
    return m_camera.GetModel();
}

/**
    Uses swscale library to convert the most recently captured frame to
    the specified format.

    @note It is currently assumed the output image will be the same size as that grabbed from
    the camera.

    @param data pointer to buffer that must be large enough to hold the data.
    @param stride number of bytes to jump between rows in data.

    @todo - conversion assumes camera image is YUYV422 - need to detect and choose format appropriately.
*/
void UnicapCapture::FrameConversion(AVPixelFormat format, uint8_t* data, int stride )
{
    const int w = m_camera.GetFrameWidth();
    const int h = m_camera.GetFrameHeight();
    const int srcStride = w*2;
    uint8_t* srcPlanes[4] = { m_buffer, 0, 0, 0 };
    int srcStrides[4] = { srcStride, 0, 0, 0 };
    uint8_t* dstPlanes[4] = { data, 0, 0, 0 };
    int dstStrides[4] = { stride, 0, 0, 0 };

    m_converter.Configure( w, h, AV_PIX_FMT_YUYV422, w, h, format );
    m_converter.Convert( srcPlanes, srcStrides, 0, h, dstPlanes, dstStrides );
}

/**
    @param data buffer into which image data will be copied - it must be large enough.
    @param stride - the number of bytes between each row in the specified 'data'
    buffer. 'stride' must be larger than the camera's image width.
*/
void UnicapCapture::ExtractLuminanceImage( uint8_t* data, int stride )
{
    unsigned char* pImg = m_buffer;
    uint32_t h = m_camera.GetFrameHeight()+1;
    uint32_t width = m_camera.GetFrameWidth();
    assert( static_cast<uint32_t>(stride) >= width );
    std::ptrdiff_t skip = stride - width;
    while ( --h )
    {
        uint32_t w = width/2;
        do
        {
            *data++ = *pImg;
            pImg += 2;
            *data++ = *pImg;
            pImg += 2;
        } while ( --w );
        data += skip;
    }
}
void UnicapCapture::ExtractRgbImage( uint8_t* dest, int stride )
{
    FrameConversion( AV_PIX_FMT_RGB24, dest, stride );
}

void UnicapCapture::ExtractBgrImage(uint8_t* dest, int stride )
{
    FrameConversion( AV_PIX_FMT_BGR24, dest, stride );
}
