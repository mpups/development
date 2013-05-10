/*
    Copyright (C) Mark Pupilli 2013, All rights reserved
*/
#ifndef __MOCK_SOCKETS_H__
#define __MOCK_SOCKETS_H__

#include "../../network/AbstractSocket.h"

#include <arpa/inet.h>

class MuxerTestSocket : public AbstractSocket
{
public:
    enum State
    {
        Type = 0,
        Size,
        Payload
    };

    MuxerTestSocket( int testPayloadSize ) : m_testPayloadSize(testPayloadSize), m_totalBytes(0), m_expected(Type) {}

    void SetBlocking( bool ) {}

    void CheckType( const char* data, std::size_t size )
    {
        EXPECT_EQ( sizeof(uint32_t), size );
        uint32_t type = ntohl( *reinterpret_cast<const uint32_t*>(data) );
        m_type = static_cast<ComPacket::Type>(type);
    }

    void CheckSize( const char* data, std::size_t size )
    {
        EXPECT_EQ( sizeof(uint32_t), size );
        uint32_t payloadSize = ntohl( *reinterpret_cast<const uint32_t*>(data) );
        if ( m_type == ComPacket::Type::Odometry )
        {
            EXPECT_EQ( m_testPayloadSize, payloadSize );
        }
    }

    /// @todo - this test is very tied to implementation (i.e. knows how the writes are broken down). Not good.
    void CheckPayload( const char* data, std::size_t size )
    {
        if ( m_type == ComPacket::Type::Odometry )
        {
            EXPECT_EQ( m_testPayloadSize, size );
        }
    }

    int Write( const char* data, std::size_t size )
    {
        switch ( m_expected )
        {
        case Type:
            CheckType( data, size );
            m_expected = Size;
            break;
        case Size:
            CheckSize( data, size );
            m_expected = Payload;
            break;
        case Payload:
            CheckPayload( data, size );
            m_expected = Type;
            break;
        default:
            ADD_FAILURE();
            break;
        }

        m_totalBytes += size;

        return size; // pretend we sent it all
    }

    // Check read functions are unused by muxer:
    int Read( char*, std::size_t )
    {
        ADD_FAILURE();
        return -1;
    }
    bool ReadyForReading( int milliseconds ) const
    {
        ADD_FAILURE();
        return false;
    }

    int m_testPayloadSize;
    int m_totalBytes;
    State m_expected;
    ComPacket::Type m_type;
};

class DemuxerTestSocket : public AbstractSocket
{
public:
    DemuxerTestSocket() {};
    virtual ~DemuxerTestSocket() {};

    virtual void SetBlocking( bool ) {}
    virtual int Write( const char*, std::size_t size ) { return size; }
    virtual int Read( char*, std::size_t size )        { return size; }
    virtual bool ReadyForReading( int milliseconds ) const { return true; };
};

/**
    This mock socket always pretends to write exactly what is asked.
    It always reports being ready to read, and always reports an io error
    (returns -1) if Read() is called.
*/
class AlwaysWriteFailReadSocket : public AbstractSocket
{
public:
    AlwaysWriteFailReadSocket() {};
    virtual ~AlwaysWriteFailReadSocket() {};

    virtual void SetBlocking( bool ) {}
    virtual int Write( const char*, std::size_t size ) { return size; }
    virtual int Read( char*, std::size_t size )        { return -1; }
    virtual bool ReadyForReading( int milliseconds ) const { return true; };
};


#endif /* __MOCK_SOCKETS_H__ */

