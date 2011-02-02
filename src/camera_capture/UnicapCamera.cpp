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
    m_time              ( -1 )
{
    if ( OpenDevice() )
    {
        assert( m_handle != 0 );
        
        unicap_format_t format;
        memset( &format, 0, sizeof(format) );

        bool success = FindFormat( 640, 480, 0x56595559, format );
        if ( success )
        {
            fprintf( stderr, "Format '%s': %dx%dx%d (%x)\n", format.identifier, format.size.width, format.size.height, format.bpp, format.fourcc );
            
            format.buffer_type = UNICAP_BUFFER_TYPE_SYSTEM;
                        
            unicap_status_t status = unicap_set_format( m_handle, &format );
            assert( SUCCESS( status ) );

            m_buffer = new uint8_t[ format.buffer_size ];
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

    delete [] m_buffer;
}

/**
    @returns true if the device was opened and setup successfully, false otherwise.
*/
bool UnicapCamera::IsAvailable() const
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
**/
void UnicapCamera::GetFrame()
{
    m_frameReady.Acquire();
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

void UnicapCamera::ExtractLuminanceImage( uint8_t* lumImg )
{
    uint32_t n = (m_width*m_height)*2/4;
    unsigned char* pImg = m_buffer;

    do
    {
        *lumImg++ = *pImg;
        pImg += 2;
        *lumImg++ = *pImg;
        pImg += 2;
    } while (--n);
}

/*
  u -= 128;
  v -= 128;
  b = y + (454*u >> 8);
  g = y - (88*u >> 8) - (183*v >> 8);
  r = y + (359*v >> 8);
  if(b<0)b=0;
  if(b>255)b=255;
  if(g<0)g=0;
  if(g>255)g=255;
  if(r<0)r=0;
  if(r>255)r=255;
*/
/*void UnicapCamera::ExtractRgbImage( uint8_t* img )
{
    uint32_t n = (m_width*m_height)*2/4;
    unsigned char* pImg = m_buffer;

    int32_t y1, y2, u, v, r, g, b, tmp_r, tmp_g, tmp_b;
    do
    {
        y1 = *pImg++;
        u  = *pImg++;
        y2 = *pImg++;
        v  = *pImg++;

        b = (454*(u-128) >> 8);
        g = (183*(v-128) >> 8) + (88*(u-128) >> 8);
        r = (359*(v-128) >> 8);

        tmp_r = r + y1;
        tmp_g = y1 - g;
        tmp_b = b + y1;        
        if ( tmp_r < 0 )   tmp_r = 0;
        if ( tmp_r > 255 ) tmp_r = 255;       
        if ( tmp_g < 0 )   tmp_g = 0;
        if ( tmp_g > 255 ) tmp_g = 255; 
        if ( tmp_b < 0 )   tmp_b = 0;
        if ( tmp_b > 255 ) tmp_b = 255; 

        *img++ = tmp_r;
        *img++ = tmp_g;
        *img++ = tmp_b;

        tmp_r = r + y2;
        tmp_g = y2 - g;
        tmp_b = b + y2;    
        if ( tmp_r < 0 )   tmp_r = 0;
        if ( tmp_r > 255 ) tmp_r = 255;       
        if ( tmp_g < 0 )   tmp_g = 0;
        if ( tmp_g > 255 ) tmp_g = 255;
        if ( tmp_b < 0 )   tmp_b = 0;
        if ( tmp_b > 255 ) tmp_b = 255;

        *img++ = tmp_r;
        *img++ = tmp_g;
        *img++ = tmp_b;
    } while (--n);
}*/

void UnicapCamera::ExtractRgbImage( uint8_t* dest )
{
    uint32_t n = (m_width*m_height)*2/4;
    unsigned char* source = m_buffer;

    do
    {
      uint8_t y1, y2, u, v;
      int c1, c2, d, dg, db, e, er, eg;
      int ir, ig, ib;
            
      y1 = *source++;
      u  = *source++;
      y2 = *source++;
      v  = *source++;
      
      c1 = (y1-16)*298;
      c2 = (y2-16)*298;
      d  = u-128;
      dg = 100 * d;
      db = 516 * d;
      e  = v-128;
      er = 409 * e;
      eg = 208 * e;

      ir = (c1 + er + 128)>>8;
      ig = (c1 - dg - eg + 128 )>>8;
      ib = (c1 + db)>>8;
      
      *dest++ = (uint8_t) ( ir > 255 ? 255 : ( ir < 0 ? 0 : ir ) );
      *dest++ = (uint8_t) ( ig > 255 ? 255 : ( ig < 0 ? 0 : ig ) );
      *dest++ = (uint8_t) ( ib > 255 ? 255 : ( ib < 0 ? 0 : ib ) );
            
      ir = (c2 + er + 128)>>8;
      ig = (c2 - dg - eg + 128 )>>8;
      ib = (c2 + db)>>8;

      *dest++ = (uint8_t) ( ir > 255 ? 255 : ( ir < 0 ? 0 : ir ) );
      *dest++ = (uint8_t) ( ig > 255 ? 255 : ( ig < 0 ? 0 : ig ) );
      *dest++ = (uint8_t) ( ib > 255 ? 255 : ( ib < 0 ? 0 : ib ) );

    } while (--n);

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
        fprintf( stderr, "\nUnicap: Opened camera with GUID %llu\n", m_guid );
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
                for ( unsigned int s=0; s<format.size_count; ++s )
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
    while ( status != STATUS_NO_MATCH );

    return success;
}

