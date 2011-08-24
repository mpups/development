/**
    Copyright (c) mark Pupilli 2011

    Tests in this file relate to TCP/IP and UDP/IP communication classes.
**/

#include <gtest/gtest.h>
#include <pthread.h>

#include "NetworkingTests.h"

#include "../../io/Socket.h"

const int MSG_SIZE = 8;
char MSG[ MSG_SIZE ] = "1234abc";

/**
    Server waits for client to connect then reads the test message from the client.
*/
void* RunServerThread( void* arg )
{
    Socket* server = (Socket*)arg;
    Socket* connection = server->Accept();

    EXPECT_FALSE( connection == 0 );
    if ( connection )
    {
        char msg[256];
        
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
    server.Bind( TEST_PORT );
    server.Listen( 0 );

    pthread_t serverThread;
    pthread_create( &serverThread, 0 , RunServerThread, (void*)&server );

    // client conencts and sends a test message
    Socket client;
    ASSERT_TRUE( client.Connect( "127.0.0.1", TEST_PORT ) );

    client.Write( MSG, MSG_SIZE );
    client.Shutdown();

    pthread_join( serverThread, 0 );
}

void TestUdp()
{
    
}

