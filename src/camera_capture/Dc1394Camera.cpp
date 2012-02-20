#include "Dc1394Camera.h"

#include <assert.h>

int32_t   Dc1394Camera::ms_objectCount = 0;
dc1394_t* Dc1394Camera::ms_video       = 0;

/**
    Enumerate cameras and opens first device found on bus.
    
    @return true if a device was successfully opened, false otherwise.
**/
bool Dc1394Camera::OpenDevice()
{
    bool rval = false;

     // open the bus if this is the first camera object:
    if ( ms_objectCount == 0 )
    {
        ms_video = dc1394_new();
        assert( ms_video != 0 );
    }

    if ( ms_video != 0 ) // Driver is not loaded
    {
        ms_objectCount += 1;

        // find a camera
        dc1394camera_list_t* list;
        int err = dc1394_camera_enumerate( ms_video, &list );
        assert( err == 0 );

        assert( list->num > 0 );
        fprintf( stderr, "\ndc1394: Found %d cameras on bus.\n", list->num );
        if ( list->num != 0 )
        {
            m_camera = dc1394_camera_new( ms_video, list->ids[0].guid );    assert( m_camera != 0 );
            m_guid = list->ids[0].guid;
            fprintf( stderr, "dc1394: Initialised camera with GUID %llu\n", m_guid );
            rval = true;        
        }

        dc1394_camera_free_list( list ); // Can free the list now    
    }

    return rval;
}

/**
    The default constructor opens the first camera found on the fire-wire bus.
**/
Dc1394Camera::Dc1394Camera()
:
    m_camera          ( 0 ),
    m_transmitting    (false),
    m_captured        (false)
{
    if ( !OpenDevice() )
    {
        return;
    }

    // Set format and iso speed (must be done before calling capture_setup)
    int err = dc1394_video_set_framerate( m_camera, DC1394_FRAMERATE_30 );      assert( err == 0 );
    err = dc1394_video_set_mode( m_camera, DC1394_VIDEO_MODE_640x480_YUV411 );  assert( err == 0 );
    err = dc1394_video_set_iso_speed( m_camera, DC1394_ISO_SPEED_400 );         assert( err == 0 );

    // Setup capture speciying number of frames that can be stored in the DMA buffer:
    err = dc1394_capture_setup( m_camera, NUM_BUFFERED_FRAMES, DC1394_CAPTURE_FLAGS_DEFAULT );  assert( err == 0 );

    // Lets find out straight away of we can get an image:
    err = dc1394_video_set_transmission( m_camera, DC1394_ON );                     assert( err == 0 );
    err = dc1394_capture_dequeue( m_camera, DC1394_CAPTURE_POLICY_WAIT, &m_frame ); assert( err == 0 );
    err = dc1394_capture_enqueue( m_camera, m_frame );                              assert( err == 0 );
    err = dc1394_video_set_transmission( m_camera, DC1394_OFF );                    assert( err == 0 );

    // We don't want to support all possible cameras and features so check some assumptions:
    assert( m_frame->data_depth == 8 );       // 8-bit images only
    if ( 
            m_frame->video_mode != DC1394_VIDEO_MODE_320x240_YUV422 &&
            m_frame->video_mode != DC1394_VIDEO_MODE_640x480_YUV411 &&
            m_frame->video_mode != DC1394_VIDEO_MODE_640x480_YUV422
          )
    {
        fprintf( stderr, "Unsupported video mode (%d)\n", m_frame->video_mode );
        dc1394_camera_free( m_camera ); // otherwise we have to power-cycle camera to use it again!
        assert(0); // unsupported video mode
        m_camera = 0;
    }
}

/**
    Closes the camera device.
*/
Dc1394Camera::~Dc1394Camera()
{
    if ( m_camera )
    {
        // Stop and close the camera:
        int err = dc1394_capture_stop( m_camera );
        assert( err == 0 );
        dc1394_camera_free( m_camera );
    
        // close the bus if this is the last camera object:
        ms_objectCount -= 1;
        assert( ms_objectCount >= 0 );
        if ( ms_objectCount == 0 )
        {
            dc1394_free( ms_video );
        }
    }
}

/**
    Query whether the camera was found and initialised successfully.

@return true if the camera was initialised succefully, false otherwise.
**/
bool Dc1394Camera::IsAvailable() const
{
    return m_camera != 0;
}

/**
    Starts capture/transmission: begins filling DMA buffer with frames.
**/
void Dc1394Camera::StartCapture()
{
    int err = dc1394_video_set_transmission( m_camera, DC1394_ON );
    assert( err == 0 );
    m_transmitting = true;
}

/**
    Stops capture/transmission: ends buffering of frames.
**/
void Dc1394Camera::StopCapture()
{
    int err = dc1394_video_set_transmission( m_camera, DC1394_OFF );
    assert( err == 0 );
    m_transmitting = false;
}

/**
    Remove the next frame from the ring buffer (blocking until one is available).

    Every call to GetFrame() should have a matching call to done frame to keep the
    DMA buffer from filling.

    Do not call GetFrame() while transmission is stopped or it will block forever.
**/
void Dc1394Camera::GetFrame()
{
    assert( m_transmitting );

    int err = dc1394_capture_dequeue( m_camera, DC1394_CAPTURE_POLICY_WAIT, &m_frame );
    assert( err == 0 );
    m_captured = true;
}

/**
    Return the last frame to the DMA buffer.

    You must do this regularly for every frame captured or the ring buffer will
    become full and frames will be discarded.
**/
void Dc1394Camera::DoneFrame()
{
    int err = dc1394_capture_enqueue( m_camera, m_frame );
    assert( err == 0 );
    m_captured = false;
}

/**
    Return width of camera image.
    @return Image width.
**/
int32_t Dc1394Camera::GetFrameWidth() const
{
    return m_frame->size[0];
}

/**
    Return height of camera image.
    @return Image height.
**/
int32_t Dc1394Camera::GetFrameHeight() const
{
    return m_frame->size[1];
}

/**
    Return time at which frame was captured.
    @return Image height.
**/
uint64_t Dc1394Camera::GetFrameTimestamp() const
{
    return m_frame->timestamp;
}

/**
    Return the Globally unique identifer of this camera (GUID).
**/
uint64_t Dc1394Camera::GetGuid() const
{
    return m_guid;
}

/**
    Access the string containing camera's vendor name.

    This string should not be modified and does not need to be freed.
**/
const char* Dc1394Camera::GetVendor() const
{
    return m_camera->vendor;
}

/**
    Access the string containing camera's model name.

    This string should not be modified and does not need to be freed.
**/
const char* Dc1394Camera::GetModel() const
{
    return m_camera->model;
}

/**
    Call between GetFrame() and DoneFrame() to extract an
    8-bit luminance (grey-scale) image from the captured frame.

    GetFrameWidth() x GetFrameHeight() pixels (bytes) will be extracted
    so @p lum must point to enough storage.
**/
void Dc1394Camera::ExtractLuminanceImage( uint8_t* lumImg )
{
    assert( m_captured ); // error to call when no frame captured

    switch( m_frame->video_mode )
    {
    
    case DC1394_VIDEO_MODE_640x480_YUV411:
    {
        uint32_t n = (m_frame->size[0] * m_frame->size[1] * 3) / 2;
        unsigned char* pImg = m_frame->image;
        unsigned char* pEnd = pImg + n;

        while( ++pImg < pEnd )
        {
            *lumImg++ = *pImg++;
            *lumImg++ = *pImg++;
        }

        break;
    }

    case DC1394_VIDEO_MODE_320x240_YUV422:
    case DC1394_VIDEO_MODE_640x480_YUV422:
    {
        uint32_t n = (m_frame->size[0] * m_frame->size[1])*2;
        unsigned char* pImg = m_frame->image + 1;
        unsigned char* pEnd = pImg + n;

        while( pImg < pEnd )
        {
            *lumImg++ = *pImg;
            pImg += 2;
            *lumImg++ = *pImg;
            pImg += 2;
        }
    }

    default:
        break;    
    }    
}

void Dc1394Camera::ExtractRgbImage( uint8_t* img )
{
    assert( 0 ); // not implemented yet!
}

void Dc1394Camera::ExtractBgrImage( uint8_t* img )
{
    assert( 0 ); // not implemented yet!
}

