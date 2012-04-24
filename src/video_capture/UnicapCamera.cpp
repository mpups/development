#include "UnicapCamera.h"

#include <stdio.h>
#include <assert.h>

/**
    This gets called in the context of unicap's capture thread when a new frame arrives.
    It then makes a copy of the capture buffer and signals a semaphore that an image is ready.
**/
void UnicapCamera::NewFrame( unicap_event_t event, unicap_handle_t handle, unicap_data_buffer_t* buffer, void *data )
{
    assert( event == UNICAP_EVENT_NEW_FRAME );

    UnicapCamera* camera = reinterpret_cast<UnicapCamera*>( data );

    if ( camera->m_captureFrame )
    {
        memcpy( camera->m_buffer, buffer->data, buffer->buffer_size );
        camera->m_time = buffer->fill_time.tv_sec * 1000000;
        camera->m_time += buffer->fill_time.tv_usec;

        camera->m_captureFrame = false;
        camera->m_frameReady.Release();
    }
}

UnicapCamera::UnicapCamera( unsigned long long guid )
:
    m_frameReady        ( 0 ),
    m_buffer            ( 0 ),
    m_time              ( -1 ),
    m_imageConversionContext (0)
{
    if ( OpenDevice() )
    {
        assert( m_handle != 0 );

        unicap_format_t format;
        memset( &format, 0, sizeof(format) );

        // Try to get YUYV422 format.
        bool success = FindFormat( 640, 480, 0x56595559, format );
        if ( success )
        {
            fprintf( stderr, "Format '%s': %dx%dx%d (%x)\n", format.identifier, format.size.width, format.size.height, format.bpp, format.fourcc );

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

    if ( m_imageConversionContext != 0 )
    {
        sws_freeContext( m_imageConversionContext );
    }
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
        m_captureFrame = true;
        status = unicap_start_capture( m_handle );
        assert( SUCCESS(status) );
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
    m_frameReady.Acquire();
    return true;
}

/**
    This releases the frame so that the driver's capture thread can return a new frame.
**/
void UnicapCamera::DoneFrame()
{
    m_captureFrame = true;
}

int32_t UnicapCamera::GetFrameWidth() const
{
    return m_width;
}

int32_t UnicapCamera::GetFrameHeight() const
{
    return m_height;
}

uint64_t UnicapCamera::GetFrameTimestamp() const
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
    @todo bug - stride is not used - should use swscale library anyway
*/
void UnicapCamera::ExtractLuminanceImage( uint8_t* data, int stride )
{
    uint32_t n = (m_width*m_height)*2/4;
    unsigned char* pImg = m_buffer;

    do
    {
        *data++ = *pImg;
        pImg += 2;
        *data++ = *pImg;
        pImg += 2;
    } while (--n);
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

    m_imageConversionContext = sws_getCachedContext( m_imageConversionContext,
                                    w, h, PIX_FMT_YUYV422, w, h, format,
                                    SWS_FAST_BILINEAR, 0, 0, 0 );

    if( m_imageConversionContext != 0 )
    {
        sws_scale( m_imageConversionContext, srcPlanes, srcStrides, 0, h, dstPlanes, dstStrides );
    }
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
bool UnicapCamera::FindFormat( int width, int height, unsigned int fourcc, unicap_format_t& result )
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
            if ( format.fourcc == fourcc )
            {
                for ( int s=0; s<format.size_count; ++s )
                {
                    if ( format.sizes[s].width == width && format.sizes[s].height == height )
                    {
                        result = format;
                        result.size.width  = width;
                        result.size.height = height;
                        success =  true;
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
    Just list all properties for debugging.
**/
void UnicapCamera::EnumerateProperties()
{
    unicap_property_t property;
    unicap_status_t status = STATUS_NO_MATCH;
    int count = 0;

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

    status = unicap_set_property_manual( m_handle, "Autogain" );
    status = unicap_set_property_value( m_handle, "Autogain", 1.0 );
    if ( SUCCESS( status ) )
    {
        fprintf( stderr, "Success: Turned on auto-gain!\n" );
    }
    else
    {
       fprintf( stderr, "Failure: Could not turn on auto-gain!\n" );
    }

    do
    {
        status = unicap_enumerate_properties( m_handle, 0, &property, count );
        fprintf ( stderr, "Property %d : %s", count, property.identifier );
        fprintf( stderr, " ( value := %f, category := %s)\n ", property.value, property.category );

        count += 1;

    } while ( SUCCESS(status) );
}

