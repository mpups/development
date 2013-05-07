#ifndef ABSTRACTSOCKET_H
#define ABSTRACTSOCKET_H

#include <cstddef>

class AbstractSocket {
public:
    AbstractSocket() {};
    virtual ~AbstractSocket() {};

    virtual void SetBlocking( bool )                       = 0;
    virtual int Write( const char*, std::size_t )         = 0;
    virtual int Read( char*, std::size_t )                = 0;
    virtual bool ReadyForReading( int milliseconds ) const = 0;
};

#endif // ABSTRACTSOCKET_H
