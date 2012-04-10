#/*
    Copyright (C) Mark Pupilli 2012, All rights reserved
*/
#ifndef __IMAGE_H__
#define __IMAGE_H__

#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

#include <iostream>

#include "ImageGeometry.h"

namespace robo
{

/**
    This provides a base class for images of generic pixel type.
    Common image types are typedef'd at the end of the header file.
*/
template <class T>
class Image
{
public:
    static const uint32_t DEFAULT_ALIGNMENT_BYTES;

    typedef T pixel_type;

    Image();
    virtual ~Image();

    // Query storage size parameters:
    uint32_t Width() const  { return m_width; };
    uint32_t Height() const { return m_height; };
    uint32_t StrideInBytes() const { return m_stride; };

    void Allocate( uint32_t width, uint32_t height );

    // Access row pointers:
    const T* operator [] ( uint32_t i ) const { return reinterpret_cast<const T*>( m_data + i*m_stride ); };
    T* operator [] ( uint32_t i ) { return reinterpret_cast<T*>( m_data + i*m_stride ); };

    const uint8_t* PixelPtr( const PixelCoord& pos ) const { return m_data + pos.y*m_stride + pos.x; };
    uint8_t* PixelPtr( const PixelCoord& pos ) { return m_data + pos.y*m_stride + pos.x; };

    void Fill( const T& value );
    void Fill( const AlignedBox& box, const T& value );
    void CopyFrom( const AlignedBox& box, const Image<T>& srcImage );

    friend void TestImage();

private:
    uint32_t m_width;
    uint32_t m_height;
    uint32_t m_stride;
    uint8_t* m_data;
};

template <class T> const uint32_t Image<T>::DEFAULT_ALIGNMENT_BYTES = 16;

template <class T>
Image<T>::Image()
:
#ifndef NDEBUG
    m_width     (0),
    m_height    (0),
    m_stride    (0),
#endif
    m_data      (0)
{
}

template <class T>
Image<T>::~Image()
{
    free( m_data );
}

/**
    Allocate (or reallocate) this Image to a new size. Any current data will be free'd and lost.
    @param width width of image in pixels.
    @param height of image in pixels.
*/
template <class T>
void Image<T>::Allocate( uint32_t width, uint32_t height )
{
    m_width = width;
    m_height = height;

    const uint32_t alignmentInBytes = DEFAULT_ALIGNMENT_BYTES;
    uint32_t rowSizeBytes = width * sizeof(T);

    // Compute the padding in bytes:
    uint32_t rem = rowSizeBytes % alignmentInBytes;

    if ( rem != 0 )
    {
        rowSizeBytes += alignmentInBytes - rem;
    }

    m_stride = rowSizeBytes;

    free( m_data );
    int err = posix_memalign( reinterpret_cast<void**>(&m_data), alignmentInBytes, m_stride*height );
    assert( err == 0 );
}

/**
    @param value set every pixel in the image to this value.
*/
template <class T>
void Image<T>::Fill( const T& value )
{
    AlignedBox box = { {0,0}, m_width, m_height };
    Fill( box, value );
}

/**
    No check is performed to see if the box fits within the
    image boundary (use AlignedBox::Clip() ).

    @param box holds the position and size of the rectangle to fill.
    @param value set every pixel in the box to this value.
*/
template <class T>
void Image<T>::Fill( const AlignedBox& box, const T& value )
{
    uint8_t* rowPtr = reinterpret_cast<uint8_t*>( (*this)[box.pos.y] + box.pos.x );
    for ( uint32_t j=box.h; j!=0; --j )
    {
        T* typedPtr = reinterpret_cast<T*>( rowPtr );
        for ( uint32_t i=box.w; i!=0; --i )
        {
            *typedPtr = value;
            ++typedPtr;
        }
        rowPtr += m_stride;
    }
}

/**
    Allocate this image to appropriate size and copy pixels from a
    region in the specified image.

    @param box defines the region to copy.
    @param srcImage the image to copy pixels from.
*/
template <class T>
void Image<T>::CopyFrom( const AlignedBox& box, const Image<T>& srcImage )
{
    Allocate( box.w, box.h );

    const uint8_t* srcRowPtr = reinterpret_cast<const uint8_t*>( srcImage[box.pos.y] + box.pos.x );
    uint8_t* dstRowPtr = m_data;
    for ( uint32_t j=m_height; j!=0; --j )
    {
        const T* srcTypedPtr = reinterpret_cast<const T*>( srcRowPtr );
        T* dstTypedPtr = reinterpret_cast<T*>( dstRowPtr );
        for ( uint32_t i=m_width; i!=0; --i )
        {
            *dstTypedPtr = *srcTypedPtr;
            ++dstTypedPtr;
            ++srcTypedPtr;
        }
        dstRowPtr += m_stride;
        srcRowPtr += srcImage.m_stride;
    }
}

} // end namespace robo

#endif /* __IMAGE_H__ */

