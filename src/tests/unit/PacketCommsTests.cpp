#include <gtest/gtest.h>

#include "../../packetcomms/PacketComms.h"

#include "../../network/AbstractSocket.h"
#include <arpa/inet.h>

void TestComPacket()
{
    ComPacket pkt;
    EXPECT_TRUE( pkt.GetType() == ComPacket::Type::Invalid );
    EXPECT_EQ( 0, pkt.GetDataSize() );
    EXPECT_EQ( nullptr, pkt.GetDataPtr() );

    constexpr int size = 6;
    uint8_t bytes[size] = "hello";
    ComPacket pkt2( ComPacket::Type::Invalid, bytes, size );
    EXPECT_EQ( size, pkt2.GetDataSize() );
    EXPECT_NE( nullptr, pkt2.GetDataPtr() );

    // Test packet contains the byte data:
    for( int i=0; i<size; ++i )
    {
        EXPECT_EQ( bytes[i], pkt2.GetData()[i] );
    }

    // Create an Odometry packet with uninitialised data:
    constexpr int size2 = 17;
    ComPacket pkt3( ComPacket::Type::Control, size2 );
    EXPECT_EQ( size2, pkt3.GetDataSize() );
    EXPECT_NE( nullptr, pkt3.GetDataPtr() );

    // Test that pkt3 gets moved (and made invalid):
    EXPECT_NE( size, size2 );
    EXPECT_EQ( ComPacket::Type::Control, pkt3.GetType() );
    pkt2 = std::move( pkt3 );
    EXPECT_EQ( ComPacket::Type::Invalid, pkt3.GetType() );
    EXPECT_EQ( size2, pkt2.GetDataSize() );

    // Test move constructor on pkt2 (which was pkt3):
    ComPacket pkt4( std::move(pkt2) );
    EXPECT_EQ( size2, pkt4.GetDataSize() );
    EXPECT_EQ( ComPacket::Type::Control, pkt4.GetType() );
    EXPECT_EQ( 0, pkt2.GetDataSize() );
    EXPECT_EQ( ComPacket::Type::Invalid, pkt2.GetType() );
}

void TestSimpleQueue()
{
    SimpleQueue q;

    // Check new queue is empty:
    EXPECT_EQ( 0, q.Size() );
    EXPECT_TRUE( q.Empty() );

    // Add a packet onto queue:
    constexpr int pktSize = 7;
    auto sptr = std::make_shared<ComPacket>( ComPacket::Type::Control, pktSize );
    EXPECT_EQ( 1, sptr.use_count() );
    q.Emplace( sptr );
    EXPECT_EQ( 1, q.Size() );
    EXPECT_EQ( 2, sptr.use_count() );

    // Check SimpleQueue::Front() returns shared_ptr to same packet:
    auto sptr2 = q.Front();
    EXPECT_EQ( 3, sptr.use_count() );
    EXPECT_TRUE( sptr == sptr2 );

    // Check queue locking:
    SimpleQueue::LockedQueue lock = q.Lock();

    // Whilts queue is not empty, and we have a lock
    // check WaitNotEmpty() returns without blocking:
    bool blocked = true;
    SimpleAsyncFunction( [&]() {
        // Wait asynchronously so failing test does not block forever:
        q.WaitNotEmpty( lock );
        blocked = false;
    });
    usleep( 1000 ); // sleep so async func has time to finish
    EXPECT_FALSE( blocked );

    // Check popping makes it empty again:
    q.Pop();
    EXPECT_EQ( 0, q.Size() );
    EXPECT_TRUE( q.Empty() );
    EXPECT_EQ( 2, sptr.use_count() );
    sptr2.reset();
}

void TestPacketMuxer()
{
    constexpr int testPayloadSize = 11;

    /// @todo - move this class:
    class MockSocket : public AbstractSocket
    {
    public:
        enum State
        {
            Type = 0,
            Size,
            Payload
        };

        MockSocket() : m_totalBytes(0), m_expected(Type) {}

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
                EXPECT_EQ( testPayloadSize, payloadSize );
            }
        }

        /// @todo - this test is very tied to implementation (i.e. knows how the writes are broken down). Not good.
        void CheckPayload( const char* data, std::size_t size )
        {
            if ( m_type == ComPacket::Type::Odometry )
            {
                EXPECT_EQ( testPayloadSize, size );
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

        int m_totalBytes;
        State m_expected;
        ComPacket::Type m_type;
    };

    MockSocket socket;
    PacketMuxer muxer( socket );

    // Transport should be ok here:
    EXPECT_TRUE( muxer.Ok() );

    usleep( 100000 );

    // Emplace payload:
    uint8_t payload[testPayloadSize];
    muxer.EmplacePacket( ComPacket::Type::Odometry, payload, testPayloadSize );

    // Post identical payload:
    ComPacket pkt( ComPacket::Type::Odometry, payload, testPayloadSize );
    muxer.PostPacket( std::move(pkt) );

    // Give the muxer time to send:
    usleep( 100000 );

    // Did it send all posted packets before exiting?
    EXPECT_EQ( muxer.GetNumPosted(), muxer.GetNumSent() );
}
