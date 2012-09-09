
#include "video_conversion.h"

#include <memory.h>

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

            // @todo fix output of colour planes
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

