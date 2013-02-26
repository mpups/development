#include "UnicapCamera.h"
#include "FourCc.h"

#include <stdio.h>
#include <assert.h>

#include "video_conversion.h"

static double milliseconds( struct timespec& t )
{
    return t.tv_sec*1000.0 + (0.000001*t.tv_nsec );
}
#include <unistd.h>

/**
    This gets called in the context of unicap's capture thread when a new frame arrives.
    It then makes a copy of the capture buffer and signals a semaphore that an image is ready.

    @todo Need different capture modes for different purposes - one should simply be a callback
    to allow access to the raw data without any copy at all.
**/
void UnicapCamera::NewFrame( unicap_event_t event, unicap_handle_t handle, unicap_data_buffer_t* buffer, void *data )
{
    assert( event == UNICAP_EVENT_NEW_FRAME );

    UnicapCamera* camera = reinterpret_cast<UnicapCamera*>( data );

    {
        camera->m_mutex.Lock();

        struct timespec t1;
        struct timespec t2;

        clock_gettime( CLOCK_MONOTONIC, &t1 );
        /// @todo This shouldn't be hard coded - wasting a lot of time everytime I forget about this!
        //halfscale_yuyv422_to_yuv420p( 640, 480, buffer->data, camera->m_buffer );
        memcpy( camera->m_buffer, buffer->data, buffer->buffer_size );
        clock_gettime( CLOCK_MONOTONIC, &t2 );

        camera->m_time = buffer->fill_time.tv_sec * 1000000;
        camera->m_time += buffer->fill_time.tv_usec;
        camera->m_frameCount += 1;

        camera->m_cond.WakeOne();
        camera->m_mutex.Unlock();
    }
}

UnicapCamera::UnicapCamera( unsigned long long guid )
:
    m_frameCount    ( 0 ),
    m_retrievedCount( 0 ),
    m_buffer        ( 0 ),
    m_time          (-1 )
{
    if ( OpenDevice() )
    {
        assert( m_handle != 0 );

        unicap_format_t format;
        memset( &format, 0, sizeof(format) );

        const char fstring[] = "YUYV";
        uint32_t fourcc = video::FourCc( fstring[0], fstring[1],fstring[2],fstring[3] );
        fprintf(stderr, "Unicap: Requesting format %s (fourcc := 0x%x)\n", fstring, fourcc );

        bool success = FindFormat( 640, 480, fourcc, format );
        if ( success )
        {
            //fprintf( stderr, "Format '%s': %dx%dx%d (%x)\n", format.identifier, format.size.width, format.size.height, format.bpp, format.fourcc );

            format.buffer_type = UNICAP_BUFFER_TYPE_SYSTEM;

            unicap_status_t status = unicap_set_format( m_handle, &format );
            assert( SUCCESS( status ) );

            int err = posix_memalign( (void**)&m_buffer, 16, format.buffer_size );

            m_width  = format.size.width;
            m_height = format.size.height;
        }
        else
        {
            fprintf( stderr, "Unicap: Could not get video format\n" );
        }
    }
}

UnicapCamera::~UnicapCamera()
{
    if ( m_handle )
    {
        unicap_close( m_handle );
    }

    free( m_buffer );
}

/**
    @returns true if the device was opened and setup successfully, false otherwise.
*/
bool UnicapCamera::IsOpen() const
{
    return m_handle != 0;
}

/**
    Registers the callback routine and starts capturing frames.    
*/
void UnicapCamera::StartCapture()
{
    int status;
    status = unicap_register_callback( m_handle, UNICAP_EVENT_NEW_FRAME, (unicap_callback_t)NewFrame, (void*)this );
    assert( SUCCESS(status) );

    if ( SUCCESS(status) )
    {
        status = unicap_start_capture( m_handle );
        assert( SUCCESS(status) );
        if ( SUCCESS(status) )
        {
            fprintf( stderr, "UnicapCamera: Capture started.\n" );
        }
    }
}

/**
    
*/
void UnicapCamera::StopCapture()
{
    unicap_stop_capture( m_handle );
}

/**
    Blocks until a frame is captured.

    You must call DoneFrame() after each call to GetFrame().

    @return true if capture was successful.
**/
bool UnicapCamera::GetFrame()
{
    m_mutex.Lock();
    while ( m_retrievedCount >= m_frameCount )
    {
        m_cond.Wait( m_mutex );
    }

    m_retrievedCount = m_frameCount;

    return true;
}

/**
    This releases the frame so that the driver's capture thread can return a new frame.
**/
void UnicapCamera::DoneFrame()
{
    m_mutex.Unlock();
}

int32_t UnicapCamera::GetFrameWidth() const
{
    return m_width;
}

int32_t UnicapCamera::GetFrameHeight() const
{
    return m_height;
}

/**
    @return the frame's timestamp in micro seconds.
*/
int64_t UnicapCamera::GetFrameTimestamp_us() const
{
    return m_time;
}

uint64_t UnicapCamera::GetGuid() const
{
    return m_guid;
}

const char* UnicapCamera::GetVendor() const
{
    return m_vendor.cStr();
}

const char* UnicapCamera::GetModel() const
{
    return m_model.cStr();
}

/**
    @param data buffer into which image data will be copied - it must be large enough.
    @param stride - the number of bytes between each row in the specified 'data'
    buffer. 'stride' must be larger than the camera's image width.
*/
void UnicapCamera::ExtractLuminanceImage( uint8_t* data, int stride )
{
    unsigned char* pImg = m_buffer;
    uint32_t h = m_height+1;
    assert( stride >= m_width );
    ptrdiff_t skip = stride - m_width;
    while ( --h )
    {
        uint32_t w = m_width/2;
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

void UnicapCamera::ExtractRgbImage( uint8_t* dest, int stride )
{
    FrameConversion( PIX_FMT_RGB24, dest, stride );
}

/**
    @note It seems that conversion to BGR is 2x faster than to RGB. BGR is also
    preferred format for OpenGL and OpenCV so this method is recommended.
*/
void UnicapCamera::ExtractBgrImage( uint8_t* dest, int stride )
{
    FrameConversion( PIX_FMT_BGR24, dest, stride );
}

/**
    @returns the pointer to the internal image buffer which holds data in the format received from the camera.

    This is only intended to be used for debugging and critical optimisations.

    @note The returned buffer is only valid between the correspoding calls to GetFrame() and DoneFrame().
*/
uint8_t* UnicapCamera::UnsafeBufferAccess() const
{
    return m_buffer;
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
void UnicapCamera::FrameConversion( PixelFormat format, uint8_t* data, int stride )
{
    const int w = m_width;
    const int h = m_height;
    const int srcStride = w*2;
    uint8_t* srcPlanes[4] = { m_buffer, 0, 0, 0 };
    int srcStrides[4] = { srcStride, 0, 0, 0 };
    uint8_t* dstPlanes[4] = { data, 0, 0, 0 };
    int dstStrides[4] = { stride, 0, 0, 0 };

    m_converter.Configure( w, h, PIX_FMT_YUYV422, w, h, format );
    m_converter.Convert( srcPlanes, srcStrides, 0, h, dstPlanes, dstStrides );
}

/**
    Opens the first device in the unicap enumeration list.
**/
bool UnicapCamera::OpenDevice()
{
    const int MAX_DEVICES = 64;
    int status = STATUS_SUCCESS;
    unicap_device_t devices[MAX_DEVICES];

    fprintf( stderr, "\nEnumerating unicap(%d.%d.%d) cameras:\n", UNICAP_MAJOR_VERSION, UNICAP_MINOR_VERSION, UNICAP_MICRO_VERSION );
    int dev_count;
    for ( dev_count = 0; SUCCESS(status) && (dev_count < MAX_DEVICES); ++dev_count )
    {
        status = unicap_enumerate_devices( 0, &devices[dev_count], dev_count );
        if ( SUCCESS(status) )
        {
            fprintf( stderr, "%d: %s, %s, %s, %llu\n", dev_count, devices[dev_count].identifier, devices[dev_count].model_name, devices[dev_count].vendor_name, devices[dev_count].model_id );
        }
    }

    unicap_open( &m_handle, &devices[0] );

    if ( m_handle )
    {
        m_guid = devices[0].model_id;
        m_vendor = GLK::String( devices[0].vendor_name );
        m_model = GLK::String( devices[0].model_name );
        fprintf( stderr, "\nUnicap: Opened camera with GUID %lu\n", m_guid );

        SetDefaultProperties();
        EnumerateProperties();
        return true;
    }
    else
    {
        fprintf( stderr, "Unicap: No cameras found.\n" );
        return false;
    }
}

/**
    Find a format which matches the spec.

    If successful then return value is true and result will contain the desired format (currently the first one found).
**/
bool UnicapCamera::FindFormat( int width, int height, uint32_t fourcc, unicap_format_t& result )
{
    bool success = false;
    int numFormats = 0;
    
    unicap_status_t status;
    unicap_format_t format;
    memset( &format, 0, sizeof(format) );
    
    do
    {
        status = unicap_enumerate_formats( m_handle, 0, &format, numFormats );
        
        if ( SUCCESS(status) )
        {
            //fprintf(stderr,"Unicap format available: %x (%x) count := %d\n",format.fourcc, fourcc, format.size_count );
            if ( format.fourcc == fourcc )
            {
                for ( int s=0; s<format.size_count; ++s )
                {
                    if ( format.sizes[s].width == width && format.sizes[s].height == height )
                    {
                        result = format;
                        result.size.width  = width;
                        result.size.height = height;
                        success = true;
                    }
                }
            }
        
            numFormats++;
        }
    }
    while ( SUCCESS(status) );

    return success;
}

/**
    Just list all available properties for debugging.
**/
void UnicapCamera::EnumerateProperties()
{
    unicap_property_t property;
    unicap_status_t status = STATUS_NO_MATCH;
    int count = 0;

    do
    {
        status = unicap_enumerate_properties( m_handle, 0, &property, count );
        if ( SUCCESS(status) )
        {
            fprintf ( stderr, "Property %d : %s", count, property.identifier );
            fprintf( stderr, " ( value := %f, category := %s)\n ", property.value, property.category );
            count += 1;
        }
    } while ( SUCCESS(status) );
}

/**
    Set some default properties (if those properties are available).

    @note These are hard-coded for the PS3 Eye camera at the moment.
*/
void UnicapCamera::SetDefaultProperties()
{
    unicap_status_t status;

    /*status = unicap_set_property_auto( m_handle, "Brightness" );
    if ( SUCCESS( status ) )
    {
        fprintf( stderr, "Success: Turned on auto-brightness!\n" );
    }
    else
    {
       fprintf( stderr, "Falure: Could not turn on auto-brightness!\n" );
    }

    status = unicap_set_property_auto( m_handle, "Exposure" );
    if ( SUCCESS( status ) )
    {
        fprintf( stderr, "Success: Turned on auto-exposure!\n" );
    }
    else
    {
       fprintf( stderr, "Falure: Could not turn on auto-exposure!\n" );
    }*/

    status = unicap_set_property_value( m_handle, "Sharpness", 0.0 );
    if ( SUCCESS( status ) )
    {
        fprintf( stderr, "Success: Set sharpness!\n" );
    }
    else
    {
       fprintf( stderr, "Falure: Could not set sharpness!\n" );
    }

    status = unicap_set_property_manual( m_handle, "Auto Gain" );
    status = unicap_set_property_value( m_handle, "Auto Gain", 1.0 );
    if ( SUCCESS( status ) )
    {
        fprintf( stderr, "Success: Turned on auto-gain!\n" );
    }
    else
    {
       fprintf( stderr, "Failure: Could not turn on auto-gain!\n" );
    }

    const double framerate_hz = 30;
    status = unicap_set_property_value( m_handle, "frame rate", framerate_hz );
    if ( SUCCESS( status ) )
    {
        fprintf( stderr, "Success: framerate set to %g Hz!\n", framerate_hz );
    }
    else
    {
        fprintf( stderr, "Failure: Could not set frame rate!\n" );
    }
}
