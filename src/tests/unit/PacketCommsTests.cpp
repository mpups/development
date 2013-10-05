#include <gtest/gtest.h>

#include "../../packetcomms/PacketComms.h"
#include "../../packetcomms/IdManager.h"
#include "MockSockets.h"

#include <memory>

TEST( packetcomms, IdManager )
{
    IdManager packetIds({ "Type1", "Type2", "Type3" });

    std::string invalid = "Invalid";
    std::string control = "Control";
    const IdManager::PacketType ctrl = packetIds.ToId( control );
    EXPECT_EQ( 0u, packetIds.ToId( invalid ) );
    EXPECT_EQ( 1u, ctrl );
    EXPECT_EQ( ctrl+1, packetIds.ToId( "Type1" ) );
    EXPECT_EQ( ctrl+2, packetIds.ToId( "Type2" ) );
    EXPECT_EQ( ctrl+3, packetIds.ToId( "Type3" ) );
    EXPECT_EQ( invalid, packetIds.ToString(0) );
    EXPECT_EQ( control, packetIds.ToString(ctrl) );
    EXPECT_EQ( "Type1", packetIds.ToString(ctrl+1) );
    EXPECT_EQ( "Type2", packetIds.ToString(ctrl+2) );
    EXPECT_EQ( "Type3", packetIds.ToString(ctrl+3) );
}

void TestComPacket()
{
    ComPacket pkt;
    EXPECT_TRUE( pkt.GetType() == IdManager::InvalidPacket );
    EXPECT_EQ( 0, pkt.GetDataSize() );
    EXPECT_EQ( nullptr, pkt.GetDataPtr() );

    constexpr int size = 6;
    uint8_t bytes[size] = "hello";
    ComPacket pkt2( IdManager::InvalidPacket, bytes, size );
    EXPECT_EQ( size, pkt2.GetDataSize() );
    EXPECT_NE( nullptr, pkt2.GetDataPtr() );

    // Test packet contains the byte data:
    for( int i=0; i<size; ++i )
    {
        EXPECT_EQ( bytes[i], pkt2.GetData()[i] );
    }

    // Create an Odometry packet with uninitialised data:
    constexpr int size2 = 17;
    ComPacket pkt3( IdManager::ControlPacket, size2 );
    EXPECT_EQ( size2, pkt3.GetDataSize() );
    EXPECT_NE( nullptr, pkt3.GetDataPtr() );

    // Test that pkt3 gets moved (and made invalid):
    EXPECT_NE( size, size2 );
    EXPECT_EQ( IdManager::ControlPacket, pkt3.GetType() );
    pkt2 = std::move( pkt3 );
    EXPECT_EQ( IdManager::InvalidPacket, pkt3.GetType() );
    EXPECT_EQ( size2, pkt2.GetDataSize() );

    // Test move constructor on pkt2 (which was pkt3):
    ComPacket pkt4( std::move(pkt2) );
    EXPECT_EQ( size2, pkt4.GetDataSize() );
    EXPECT_EQ( IdManager::ControlPacket, pkt4.GetType() );
    EXPECT_EQ( 0, pkt2.GetDataSize() );
    EXPECT_EQ( IdManager::InvalidPacket, pkt2.GetType() );
}

void TestSimpleQueue()
{
    SimpleQueue q;

    // Check new queue is empty:
    EXPECT_EQ( 0, q.Size() );
    EXPECT_TRUE( q.Empty() );

    // Add a packet onto queue:
    constexpr int pktSize = 7;
    auto sptr = std::make_shared<ComPacket>( IdManager::ControlPacket, pktSize );
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

void TestPacketMuxerExitsCleanly()
{
    AlwaysFailSocket mockSocket;
    PacketMuxer muxer( mockSocket, {} );
    while ( muxer.Ok() ) {}
}

void TestPacketMuxer()
{
    constexpr int testPayloadSize = 11;
    MuxerTestSocket socket( testPayloadSize );
    PacketMuxer muxer( socket, {"MockPacket"} );

    // Transport should be ok here:
    EXPECT_TRUE( muxer.Ok() );

    usleep( 100000 );

    // Emplace payload:
    uint8_t payload[testPayloadSize];
    muxer.EmplacePacket( "MockPacket", payload, testPayloadSize );

    // Post identical payload:
    muxer.EmplacePacket( "MockPacket", payload, testPayloadSize );

    // Give the muxer time to send:
    usleep( 100000 );

    // Did it send all posted packets before exiting?
    EXPECT_EQ( muxer.GetNumPosted(), muxer.GetNumSent() );
}

void TestDemuxerExitsCleanly()
{
    AlwaysFailSocket socket;
    PacketDemuxer demuxer( socket, {} );

    while ( demuxer.Ok() )
    {
    }
}

void TestPacketDemuxer()
{
    AlwaysFailSocket socket;
    PacketDemuxer demuxer( socket, {} );
}
