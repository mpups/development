#include "UnicapCamera.h"
#include "FourCc.h"

#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <memory.h>

#include <functional>
#include <iostream>

UnicapCamera::UnicapCamera( uint64_t guid )
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
}

/**
    @returns true if the device was opened and setup successfully, false otherwise.
*/
bool UnicapCamera::IsOpen() const
{
    return m_handle != 0;
}

void UnicapCamera::NewFrame( unicap_event_t event, unicap_handle_t handle, unicap_data_buffer_t* buffer, void *data )
{
    assert( event == UNICAP_EVENT_NEW_FRAME );

    timespec fillTime;
    fillTime.tv_sec = buffer->fill_time.tv_sec;
    fillTime.tv_nsec = buffer->fill_time.tv_usec * 1000;

    UnicapCamera* camera = reinterpret_cast<UnicapCamera*>( data );
    if ( camera->m_captureCallback == false )
    {
        std::cout << "Error: no capture callback set - did you call StartCapture() before SetCaptureCallback()?" << std::endl;
    }
    camera->m_captureCallback( buffer->data, fillTime );
}

/**
    Registers the callback routine and starts capturing frames.    
*/
void UnicapCamera::StartCapture()
{
    assert( m_captureCallback );
    const int registered = unicap_register_callback( m_handle, UNICAP_EVENT_NEW_FRAME, (unicap_callback_t)NewFrame, (void*)this );
    assert( SUCCESS(registered) );

    if ( SUCCESS(registered) )
    {
        const int started = unicap_start_capture( m_handle );
        assert( SUCCESS(started) );
        if ( SUCCESS(started) )
        {
            std::clog << "UnicapCamera: Capture started." << std::endl;
        }
    }
}

/**
    
*/
void UnicapCamera::StopCapture()
{
    unicap_stop_capture( m_handle );
}

int32_t UnicapCamera::GetFrameWidth() const
{
    return m_width;
}

int32_t UnicapCamera::GetFrameHeight() const
{
    return m_height;
}

uint64_t UnicapCamera::GetGuid() const
{
    return m_guid;
}

const char* UnicapCamera::GetVendor() const
{
    return m_vendor.c_str();
}

const char* UnicapCamera::GetModel() const
{
    return m_model.c_str();
}

/**
    @returns the pointer to the internal image buffer which holds data in the format received from the camera.

    This is only intended to be used for debugging and critical optimisations.

    @note The returned buffer is only valid between the correspoding calls to GetFrame() and DoneFrame().
*/
std::size_t UnicapCamera::GetFormatBufferSize() const
{
    unicap_format_t format;
    int status = unicap_get_format( m_handle, &format );
    if ( status == STATUS_SUCCESS )
    {
        return format.buffer_size;
    }
    else
    {
        return 0;
    }
}

void UnicapCamera::SetCaptureCallback( CaptureFunction&& callback )
{
    m_captureCallback = callback;
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
        m_vendor = std::string( devices[0].vendor_name );
        m_model = std::string( devices[0].model_name );
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
    }*/

    status = unicap_set_property_auto( m_handle, "Exposure" );
    if ( SUCCESS( status ) )
    {
        fprintf( stderr, "Success: Turned on auto-exposure!\n" );
    }
    else
    {
       fprintf( stderr, "Falure: Could not turn on auto-exposure!\n" );
    }

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

    const double framerate_hz = 60;
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
