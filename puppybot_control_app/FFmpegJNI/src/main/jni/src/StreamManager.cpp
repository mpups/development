#include "StreamManager.h"
#include "TcpVideoStream.h"

StreamManager::StreamManager()
{
}

StreamManager::~StreamManager()
{
}

std::size_t StreamManager::GetStreamCount() const
{
	return m_stream.size();
}

/**
 	 Construct a new stream object.

 	 If it successfully initialises a video stream then add it to internal stream container.
 	 Otherwise the object is destroyed and not added to the set of streams.

 	 @param socket Reference to a valid/connected socket.
 * */
int StreamManager::NewStream( TcpSocket& socket, int timeoutInSeconds )
{
	m_stream.emplace_back( new TcpVideoStream( socket, timeoutInSeconds ) );

	int index;
	if ( m_stream.back()->IsInitialised() == true )
	{
		index = m_stream.size() - 1;
	}
	else
	{
		m_stream.pop_back();
		index = -1;
	}

	return index;
}

void StreamManager::DeleteStream( int i )
{
	if ( i < m_stream.size() )
	{
		m_stream[i].reset();
		m_stream.erase( m_stream.begin() + i );
	}
}

StreamManager::StreamPtr& StreamManager::operator[] ( int i )
{
	return m_stream[i];
}
