/*
    Copyright (C) Mark Pupilli 2013, All rights reserved
*/
#ifndef __STREAM_MANAGER_H__
#define __STREAM_MANAGER_H__

#include <memory>
#include <vector>

class TcpVideoStream;
class TcpSocket;

class StreamManager
{
public:
	typedef std::unique_ptr<TcpVideoStream> StreamPtr;

	StreamManager();
	virtual ~StreamManager();

	std::size_t GetStreamCount() const;
	int NewStream( TcpSocket& socket, int timeoutInSeconds );
	void DeleteStream( int i );
	StreamPtr& operator[] ( int i );

protected:

private:
	std::vector<StreamPtr> m_stream;
};

#endif /* __STREAM_MANAGER_H__ */

