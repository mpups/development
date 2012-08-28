#include "RobotServer.h"

const int IMG_WIDTH  = 320;
const int IMG_HEIGHT = 240;

#include <time.h>

static double milliseconds( struct timespec& t )
{
    return t.tv_sec*1000.0 + (0.000001*t.tv_nsec );
}

/**
    Setup a robot server with specified TCP and serial ports.
    
    TCP is used for a remote tele-link and the serial port is used for differential drive control (if available).
**/
RobotServer::RobotServer( int tcpPort, const char* motorSerialPort )
:
    m_serialPort( motorSerialPort ),
    m_drive ( 0 ),
    m_motors( 0 ),
    m_server( 0 ),
    m_con   ( 0 ),
    m_camera( 0 )
{   
    // Setup a server socket for receiving client commands:
    m_server = new TcpSocket();
    if ( m_server->Bind( tcpPort ) == false )
    {
        delete m_server;
        m_server = 0;
    }
}

RobotServer::~RobotServer()
{
    delete m_camera;
    delete m_con;
    delete m_server;
    delete m_server;
    delete m_drive;
    delete m_motors;
}

/**
    Blocks until robot gets a connection.
**/
bool RobotServer::Listen()
{
    if ( m_server )
    {
        fprintf( stderr, "Waiting for new connection...\n" );
        m_server->Listen( 0 ); // Wait for connection - no queue
        m_con = m_server->Accept(); // Create connection
        m_con->SetBlocking( false );

        PostConnectionSetup();
        return true;
    }
    else
    {
        fprintf( stderr, "Error: Server socket invalid.\n" );
        return false;
    }
}

/**
    Perform post conection processing.
    
    Attempts to access camera and wheels.
**/
void RobotServer::PostConnectionSetup()
{
    // Setup comms to motors:
    m_motors = new MotionMind( m_serialPort.cStr() );
    if ( m_motors->Available() )
    {
        m_drive = new DiffDrive( *m_motors );
        float amps = 1.5f;
        int32_t currentLimit = roundf( amps/0.02f );
        int32_t pwmLimit = (72*1024)/120; // motor voltage / battery voltage
        
        m_motors->WriteRegister( 1, MotionMind::AMPSLIMIT, currentLimit );
        m_motors->WriteRegister( 2, MotionMind::AMPSLIMIT, currentLimit );
        m_motors->WriteRegister( 1, MotionMind::PWMLIMIT, pwmLimit );
        m_motors->WriteRegister( 2, MotionMind::PWMLIMIT, pwmLimit );
    }
    else
    {
        delete m_motors;
        m_motors = 0;
    }

    // Setup camera:
    m_camera = new UnicapCamera();
    size_t imageBufferSize = m_camera->GetFrameWidth() * m_camera->GetFrameHeight() * sizeof(uint8_t);
    if ( m_camera->IsOpen() )
    {
        m_camera->StartCapture();
        int err = posix_memalign( (void**)&m_lum, 16, imageBufferSize );
        assert( err == 0 );
    }
    else
    {
        delete m_camera;
        m_camera = 0;
    }
}

/**
    Cleans up resources once the comms loop has finished.
**/
void RobotServer::PostCommsCleanup()
{
    // These must be deleted in this order:
    delete m_drive;
    delete m_motors;
    m_drive = 0;
    m_motors = 0;
    
    if ( m_camera )
    {
        m_camera->StopCapture();
        delete m_camera;
        free( m_lum );
    }
}

/**
    Runs the robot's comms loop until the connection terminates or fails.
**/
void RobotServer::RunCommsLoop()
{
    // Setup a TeleJoystick object:
    TeleJoystick* teljoy = 0;
    if ( m_con )
    {
        Ipv4Address clientAddress;
        m_con->GetPeerAddress( clientAddress );

        {
            std::string name;
            clientAddress.GetHostName( name );
            fprintf( stderr, "Client %s connected to robot.\n", name.c_str() );
        }

        teljoy = new TeleJoystick( *m_con, m_drive ); // Will start receiving and processing remote joystick cammands immediately.
        teljoy->Go();

        GLK::Thread::Sleep( 100 );
        fprintf( stderr, "running: %d\n", teljoy->IsRunning() );

        // Start capturing and transmitting images:
        if ( m_camera )
        {
            StreamVideo( *teljoy );
        }
        else
        {
            while ( teljoy->IsRunning() ) {
                sleep(100);
            }
        }

        fprintf( stderr, "Control terminated\n" );
    } // end if

    delete teljoy;

    PostCommsCleanup();
}

// gcc for Arm defines the following if you set -mfpu=neon
#ifdef __ARM_NEON__

#include <arm_neon.h>

/**
    Take image data in yuyv422 format and convert to yuv420p while reducing the image
    dimensions by half.

    yuv420p format is a planar 12-bit-per-pixel format where the chroma planes are
    sub-sampled by 2 (each 2x2 block of luminance has a single u,v chrominance pair).
    The first plane is the full resolution chroma plane, follwed by the u plane and
    finally the v plane. This format is commonly required as an input for video codecs.

    This function expects the separate planes to all be tightly packed into the src and
    dst buffers. This might not lead to optimal alignment for some image dimensions.

    @param w the input width in pixels.
    @param h the input height in pixels.
    @srcBuffer input yuv420p image of dimension wxh.
    @dstBuffer output yuv420p image of dimension (w/2)x(h/2).
*/
void halfscale_yuyv422_to_yuv420p( int w, int h, uint8_t* srcBuffer, uint8_t* dstBuffer )
{
    uint8_t* lumaDst = dstBuffer;
    uint8_t* chromaDst = dstBuffer + ((w*h)/4);

    const int rowBytes = w*2;
    int r = (h/2) + 1;
    while ( --r )
    {
        int c = (w/16) + 1;
        while ( --c )
        {
            // Read and unpack 32 bytes (16 pixels) at a time:
            // y1 u1 y2 v1 y3 u2 y4 v2 y5 u3 y6 v3 y7 u4 y8 v4 y9 u5 y10 v5 y11 u6 y12 v6 y13 u7 y14 v7 y15 u8 y16 v8 ->
            // [0] = y1 y3 y5 y7 y9 y11 y13 y15
            // [1] = u1 u2 u3 u4 u5 u6 u7 u8
            // [2] = y2 y4 y6 y8 y10 y12 y14 y16
            // [3] = v1 v2 v3 v4 v5 v6 v7 v8
            uint8x8x4_t row1_yuyv_lanes = vld4_u8( srcBuffer );
            uint8x8x4_t row2_yuyv_lanes = vld4_u8( srcBuffer + rowBytes ); // process corresponding pixels from next row simultaneously

            // Average adjacent luminance components using halving add: 
            uint8x8_t row1_luma = vrhadd_u8( row1_yuyv_lanes.val[0], row1_yuyv_lanes.val[2] ); // ([0] + [2]) / 2 -> Y1 Y2 Y3 Y4 Y5 Y6 Y7 Y8
            uint8x8_t row2_luma = vrhadd_u8( row2_yuyv_lanes.val[0], row2_yuyv_lanes.val[2] );

            // Average chroma components using pair-wise add:
            uint16x4_t row1_chroma_u_16 = vpaddl_u8( row1_yuyv_lanes.val[1] ); // [1] -> U1 U2 U3 U4 (U1 = (u1+u2)/2, etc)
            uint16x4_t row1_chroma_v_16 = vpaddl_u8( row1_yuyv_lanes.val[3] ); // [3] -> V1 V2 V3 V4
            uint16x4_t row2_chroma_u_16 = vpaddl_u8( row2_yuyv_lanes.val[1] );
            uint16x4_t row2_chroma_v_16 = vpaddl_u8( row2_yuyv_lanes.val[3] );

            // Combine u anv v into one vector:
            uint16x8_t row1_chroma_uv_16 = vcombine_u16( row1_chroma_u_16, row1_chroma_v_16 ); // U1 U2 U3 U4 V1 V2 V3 V4
            uint16x8_t row2_chroma_uv_16 = vcombine_u16( row2_chroma_u_16, row2_chroma_v_16 );

            // Halve and truncate chroma back to 8-bits:
            // vrshrn does rounding but takes 1 more cycle - not worth it?
            uint8x8_t row1_chroma_uv_8 = vshrn_n_u16( row1_chroma_uv_16, 1 ); // (U1 U2 U3 U4 V1 V2 V3 V4) / 2
            uint8x8_t row2_chroma_uv_8 = vshrn_n_u16( row2_chroma_uv_16, 1 );

            // Now average all data for the two rows with halving add:
            uint8x8x2_t yuv_out; // this type ensures we get an adjacent pair of registers
            yuv_out.val[0] = vrhadd_u8( row1_luma, row2_luma ); // luma result: y1 y2 y3 y4 y5 y6 y7 y8
            yuv_out.val[1] = vrhadd_u8( row1_chroma_uv_8, row2_chroma_uv_8 ); // chroma result: u1 u2 u3 u4 v1 v2 v3 v4

            // Write luma to first plane:
            vst1_u8( lumaDst, yuv_out.val[0] );

            // First write the chroma planes without vertical subsampling:
            //vst1_u8( chromaDst, yuv_out.val[1] );

            lumaDst += 8;
            chromaDst += 8;

            srcBuffer += 32;
        }

        srcBuffer += rowBytes;
    }

    // Set no colour in chroma planes:
    memset( dstBuffer + ((w*h)/4), 127, (w*h)/8 );
}

/**
    Neon optimised half-scale yuyv422.
    Take image data in yuyv422 format and reduce the image dimensions by half.

    @param w width in pixels - must be multiple of 16
    @param h height in pixels
    @param srcBuffer pointer to yuyv422 image data - must be 16-byte/128-bit aligned
    @param dstBuffer pointer to storage for yuyv422 result - must be 16-byte/128-bit aligned

    @note The src and dst buffers are allowed to be the same buffer (works correctly in place).
*/
void halfscale_yuyv422( int w, int h, uint8_t* srcBuffer, uint8_t* dstBuffer )
{
    uint8_t perm[] = { 0,8,1,12,2,9,3,13,4,10,5,14,6,11,7,15 };
    uint8x8_t yuyv422_store_permutation1 = vld1_u8( perm );
    uint8x8_t yuyv422_store_permutation2 = vld1_u8( perm+8 );

    const int rowBytes = w*2;
    int r = (h/2) + 1;
    while ( --r )
    {

        int c = (w/16) + 1;
        while ( --c )
        {
            // Read and unpack 32 bytes (16 pixels) at a time:
            // y1 u1 y2 v1 y3 u2 y4 v2 y5 u3 y6 v3 y7 u4 y8 v4 y9 u5 y10 v5 y11 u6 y12 v6 y13 u7 y14 v7 y15 u8 y16 v8 ->
            // [0] = y1 y3 y5 y7 y9 y11 y13 y15
            // [1] = u1 u2 u3 u4 u5 u6 u7 u8
            // [2] = y2 y4 y6 y8 y10 y12 y14 y16
            // [3] = v1 v2 v3 v4 v5 v6 v7 v8
            uint8x8x4_t row1_yuyv_lanes = vld4_u8( srcBuffer );
            uint8x8x4_t row2_yuyv_lanes = vld4_u8( srcBuffer + rowBytes ); // process corresponding pixels from next row simultaneously
            
            // Average adjacent luminance components using halving add: 
            uint8x8_t row1_luma = vrhadd_u8( row1_yuyv_lanes.val[0], row1_yuyv_lanes.val[2] ); // ([0] + [2]) / 2 -> Y1 Y2 Y3 Y4 Y5 Y6 Y7 Y8
            uint8x8_t row2_luma = vrhadd_u8( row2_yuyv_lanes.val[0], row2_yuyv_lanes.val[2] );

            // Average chroma components using pair-wise add:
            uint16x4_t row1_chroma_u_16 = vpaddl_u8( row1_yuyv_lanes.val[1] ); // [1] -> U1 U2 U3 U4 (U1 = (u1+u2)/2, etc)
            uint16x4_t row1_chroma_v_16 = vpaddl_u8( row1_yuyv_lanes.val[3] ); // [3] -> V1 V2 V3 V4
            uint16x4_t row2_chroma_u_16 = vpaddl_u8( row2_yuyv_lanes.val[1] );
            uint16x4_t row2_chroma_v_16 = vpaddl_u8( row2_yuyv_lanes.val[3] );

            // Combine u anv v into one vector:
            uint16x8_t row1_chroma_uv_16 = vcombine_u16( row1_chroma_u_16, row1_chroma_v_16 ); // U1 U2 U3 U4 V1 V2 V3 V4
            uint16x8_t row2_chroma_uv_16 = vcombine_u16( row2_chroma_u_16, row2_chroma_v_16 );

            // Halve and truncate chroma back to 8-bits:
            // vrshrn does rounding but takes 1 more cycle - not worth it?
            uint8x8_t row1_chroma_uv_8 = vshrn_n_u16( row1_chroma_uv_16, 1 ); // (U1 U2 U3 U4 V1 V2 V3 V4) / 2
            uint8x8_t row2_chroma_uv_8 = vshrn_n_u16( row2_chroma_uv_16, 1 );

            // Now average all data for the two rows with halving add:
            uint8x8x2_t yuv_out; // this type ensures we get an adjacent pair of registers
            yuv_out.val[0] = vrhadd_u8( row1_luma, row2_luma ); // luma result: y1 y2 y3 y4 y5 y6 y7 y8
            yuv_out.val[1] = vrhadd_u8( row1_chroma_uv_8, row2_chroma_uv_8 ); // chroma result: u1 u2 u3 u4 v1 v2 v3 v4

            // Use table-lookup instructions to permute the vectors:
            uint8x8x2_t yuyv422_out;
            yuyv422_out.val[0] = vtbl2_u8( yuv_out, yuyv422_store_permutation1 );
            yuyv422_out.val[1] = vtbl2_u8( yuv_out, yuyv422_store_permutation2 );

            srcBuffer += 32;

            // Finally store the results:
            uint8x16_t out = vcombine_u8 ( yuyv422_out.val[0], yuyv422_out.val[1] ); // hopefully a NOP
            vst1q_u8( dstBuffer, out );
            dstBuffer += 16;
        }

        srcBuffer += rowBytes;
    }
}

#else

/**
    Take image data in yuyv422 format and reduce the image dimensions by half.

    The input width and height must be exactly divisible by 4.

    @param w the input width in pixels (must be exactly divisible by 2).
    @param h the input height in pixels (must be exactly divisible by 2).
    @param srcBuffer input image of dimension wxh.
    @param dstBuffer output image of dimension (w/2)x(h/2).

    @note The src and dst buffers are allowed to be the same buffer (works correctly in place).
*/
void halfscale_yuyv422( int w, int h, uint8_t* srcBuffer, uint8_t* dstBuffer )
{
    uint8_t* const pSrc = srcBuffer;
    uint8_t* const pDst = dstBuffer;

    // On first pass we'll scale horizontally:

    int r = h + 1;
    while ( --r )
    {

        __builtin_prefetch( srcBuffer+(w/4), 0, 0 );
        __builtin_prefetch( dstBuffer+(w/4), 1, 0 );

        int c = (w/4) + 1;
        while ( --c )
        {
            // Read 8 bytes (4 pixels) at a time:
            uint16_t y1,u1,y2,v1;
            uint16_t y3,u2,y4,v2;

            y1 = srcBuffer[0];
            u1 = srcBuffer[1];
            y2 = srcBuffer[2];
            v1 = srcBuffer[3];
            y3 = srcBuffer[4];
            u2 = srcBuffer[5];
            y4 = srcBuffer[6];
            v2 = srcBuffer[7];

            dstBuffer[0] = (y1+y2)/2;
            dstBuffer[1] = (u1+u2)/2;
            dstBuffer[2] = (y3+y4)/2;
            dstBuffer[3] = (v1+v2)/2;

            srcBuffer += 8;
            dstBuffer += 4;
        }
    }

    // Second pass scale vertically:

    // reset data pointers:
    srcBuffer = pSrc;
    dstBuffer = pDst;

    int rowBytes = w;

    r = (h/2) + 1;
    while ( --r )
    {
        int c = rowBytes + 1;
        while ( --c )
        {
            uint16_t c1,c2;
            c1 = srcBuffer[0];
            c2 = srcBuffer[rowBytes];
            dstBuffer[0] = (c1+c2)/2;
            srcBuffer += 1;
            dstBuffer += 1;
        }

        srcBuffer += rowBytes;
    }
}

#endif

/**
    Assumptions:
        m_con is not null.
*/
void RobotServer::StreamVideo( TeleJoystick& joy )
{
    assert( m_con );

    // Create a video writer object that uses socket IO:
    FFMpegSocketIO videoIO( *m_con, true );
    LibAvWriter streamer( videoIO );

    // Setup an MPEG4 video stream for half-size video:
    int streamWidth = 320; // m_camera->GetFrameWidth()
    int streamHeight = 240; // m_camera->GetFrameHeight()
    streamer.AddVideoStream( streamWidth, streamHeight, 30, LibAvWriter::FourCc( 'F','M','P','4' ) );

    struct timespec t1;
    struct timespec t2;
    struct timespec t3;

    // Create a buffer for full-size image in format YUV420P:
    uint8_t* yuv420p;
    int w = m_camera->GetFrameWidth();
    int h = m_camera->GetFrameHeight();
    int err = posix_memalign( (void**)&yuv420p, 16, (3 * w * h) / 2 );
    assert( err == 0 );

    // Set no colour in chroma planes:
    memset( yuv420p, 127, (3*w*h)/2 );

    bool sentOk = true;
    clock_gettime( CLOCK_MONOTONIC, &t1 );
    while ( sentOk && joy.IsRunning() && m_camera->GetFrame() )
    {
        clock_gettime( CLOCK_MONOTONIC, &t2 );

        //halfscale_yuyv422( w, h, m_camera->UnsafeBufferAccess(), m_camera->UnsafeBufferAccess() );
        halfscale_yuyv422_to_yuv420p( w, h, m_camera->UnsafeBufferAccess(), m_camera->UnsafeBufferAccess() );

        clock_gettime( CLOCK_MONOTONIC, &t3 );

        //sentOk = streamer.PutYUYV422Frame( m_camera->UnsafeBufferAccess(), w/2, h/2 );
        sentOk = streamer.PutYUV420PFrame( m_camera->UnsafeBufferAccess(), w/2, h/2 ); // yuv420p is native format for MPEG4

        m_camera->DoneFrame();
        sentOk &= videoIO.GetAVIOContext()->error >= 0;

        double grabTime = milliseconds(t2) - milliseconds(t1);
        double extractTime = milliseconds(t3) - milliseconds(t2);
        fprintf( stderr, "%f %f %f %f %f\n", grabTime, extractTime, streamer.lastConvertTime_ms, streamer.lastEncodeTime_ms, streamer.lastPacketWriteTime_ms );
        clock_gettime( CLOCK_MONOTONIC, &t1 );
    }
}

