#include <gtest/gtest.h>

#include "../../packetcomms/PacketComms.h"

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
