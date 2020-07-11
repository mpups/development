/**
    Copyright (c) mark Pupilli 2011

    Tests in this file relate to TCP/IP and UDP/IP communication classes.
**/

#include <gtest/gtest.h>
#include <pthread.h>

#include "NetworkingTests.h"

#include "../../network/Socket.h"
#include "../../network/TcpSocket.h"
#include "../../network/UdpSocket.h"
#include "../../network/Ipv4Address.h"

const int MSG_SIZE = 8;
const char MSG[ MSG_SIZE ] = "1234abc";
const char UDP_MSG[] = "Udp connection-less Datagram!";

/*
    Server waits for client to connect then reads the test message from the client.
*/
void* RunTcpServerThread( void* arg )
{
    TcpSocket* server = (TcpSocket*)arg;
    TcpSocket* connection = server->Accept();

    EXPECT_FALSE( connection == 0 );
    if ( connection )
    {
        char msg[256] = "";
        
        EXPECT_TRUE( connection->IsValid() );
        int bytes = connection->Read( msg, MSG_SIZE );
        EXPECT_EQ( bytes, MSG_SIZE );
        EXPECT_STREQ( msg, MSG );

        connection->Shutdown();
        delete connection;
    }

	return 0;
}

void TestTcp()
{
    const int TEST_PORT = 2000;
    TcpSocket server;
    EXPECT_TRUE( server.IsValid() );
    server.Bind( TEST_PORT );
    server.Listen( 0 );

    // Create a server thread for testing:
    pthread_t serverThread;
    pthread_create( &serverThread, 0 , RunTcpServerThread, (void*)&server );

    // client connects and sends a test message
    TcpSocket client;
    ASSERT_TRUE( client.Connect( "localhost", TEST_PORT ) );

    client.Write( MSG, MSG_SIZE );
    client.Shutdown();

    pthread_join( serverThread, 0 );
}

/*
    Server waits for messages from client and checks they are correct.
*/
void* RunUdpServerThread( void* arg )
{
    UdpSocket* server = (UdpSocket*)arg;
    
    char msg[256] = "";

    // First read the connection-less udp message:
    const int udpMsgSize = strlen(UDP_MSG) + 1;
    int bytes = server->Read( msg, udpMsgSize );
    EXPECT_EQ( bytes, udpMsgSize );
    EXPECT_STREQ( msg, UDP_MSG );

    // Then read the connected message:
    bytes = server->Read( msg, MSG_SIZE );
    EXPECT_EQ( bytes, MSG_SIZE );
    EXPECT_STREQ( msg, MSG );

	return 0;
}

void TestUdp()
{
    const int TEST_PORT = 3000;
    UdpSocket server;
    EXPECT_TRUE( server.IsValid() );
    server.Bind( TEST_PORT );

    // Create a server thread for testing:
    pthread_t serverThread;
    pthread_create( &serverThread, 0 , RunUdpServerThread, (void*)&server );

    // Test connection-less datagram:
    Ipv4Address localhost( "127.0.0.1", TEST_PORT );
    ASSERT_TRUE( localhost.IsValid() );
    UdpSocket client;
    int bytesSent = client.SendTo( localhost, UDP_MSG, strlen(UDP_MSG)+1 );
    ASSERT_EQ( strlen(UDP_MSG)+1, bytesSent );

    // Test datagram to a connection:
    client.Connect( "127.0.0.1", TEST_PORT );
    client.Write( MSG, MSG_SIZE );

    pthread_join( serverThread, 0 );

    client.Shutdown();
}

void TestIpv4Address()
{
    const int TEST_PORT = 3000;
    Ipv4Address localhost( "localhost", TEST_PORT );
    ASSERT_TRUE( localhost.IsValid() );

    std::string hostName;
    localhost.GetHostName( hostName );
    EXPECT_STREQ( "localhost", hostName.c_str() );
    std::string hostIP;
    localhost.GetHostAddress( hostIP );
    EXPECT_STREQ( "127.0.0.1", hostIP.c_str() );

    EXPECT_EQ( TEST_PORT, localhost.GetPort() );

    Ipv4Address nonsense( "@nonsense.ww.arg.?", 120 );
    ASSERT_FALSE( nonsense.IsValid() );

    Ipv4Address uninitialised;
    ASSERT_FALSE( uninitialised.IsValid() );

    Ipv4Address copy( localhost );
    ASSERT_TRUE( localhost.IsValid() );
}

