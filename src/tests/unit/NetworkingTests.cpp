/**
    Copyright (c) mark Pupilli 2011

    Tests in this file relate to TCP/IP and UDP/IP communication classes.
**/

#include <gtest/gtest.h>
#include <pthread.h>

#include "NetworkingTests.h"

#include "../../io/Socket.h"
#include "../../io/UdpSocket.h"

const int MSG_SIZE = 8;
const char MSG[ MSG_SIZE ] = "1234abc";
const char UDP_MSG[] = "Udp connection-less Datagram!";

/**
    Server waits for client to connect then reads the test message from the client.
*/
void* RunTcpServerThread( void* arg )
{
    Socket* server = (Socket*)arg;
    Socket* connection = server->Accept();

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
    Socket server;
    EXPECT_TRUE( server.IsValid() );
    server.Bind( TEST_PORT );
    server.Listen( 0 );

    // Create a server thread for testing:
    pthread_t serverThread;
    pthread_create( &serverThread, 0 , RunTcpServerThread, (void*)&server );

    // client connects and sends a test message
    Socket client;
    ASSERT_TRUE( client.Connect( "127.0.0.1", TEST_PORT ) );

    client.Write( MSG, MSG_SIZE );
    client.Shutdown();

    pthread_join( serverThread, 0 );
}

/**
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
    UdpSocket client;
    int bytesSent = client.SendTo( "127.0.0.1", TEST_PORT, UDP_MSG, strlen(UDP_MSG)+1 );
    ASSERT_EQ( strlen(UDP_MSG)+1, bytesSent );

    // Test datagram to a connection:
    client.Connect( "127.0.0.1", TEST_PORT );
    client.Write( MSG, MSG_SIZE );

    pthread_join( serverThread, 0 );

    client.Shutdown();
}

