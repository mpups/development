/*
    Copyright (C) Mark Pupilli 2013, All rights reserved
*/
#ifndef __ASYNC_LOOPER_H__
#define __ASYNC_LOOPER_H__

#include <functional>
#include <chrono>
#include <thread>
#include <iostream>
#include <iomanip>

#include <pthread.h>
#include <assert.h>
#include <unistd.h>

/**
    Creates a thread which loops and repeatedly calls astd::function in a
    separate thread until the object is destroyed.

    The function is moved in on construction
    and no further access is provided to it. A new thread is spawned
    immediately and the function is called indefinitely in that thread's
    context.

    There is no way to kill the thread prematurely - the function must
    exit of its own accord.

    The internally spawned thread is not joined until the destructor is
    called. The call to the destructor will block until the thread exits
    (function returns).

    The function is guaranteed to be called at least once.

    Example - execute a lambda in the context of a new thread:
\code
        AsyncLooperFunction( 10, []() {
            // called aproxiimately 10 times a second
        });
\endcode

    Objects can be moved, but not copied.
*/
class AsyncLooper
{
public:
    typedef std::function< void(void) > Type;

    AsyncLooper( const AsyncLooper& ) = delete;
    AsyncLooper( AsyncLooper&& ) = delete;

    /**
        Move in a function object and start a thread that will call this
        function in a separate thread at the specified frequency.
    */
    AsyncLooper( float hertz, Type&& f )
    :
        m_asyncFunction ( std::move(f) ),
        m_createError   ( pthread_create( &m_thread, nullptr, AsyncLooper::Go, this ) ),
        m_Hz            ( hertz ),
        m_go            ( true )
    {
        assert( ThreadWasCreated() );
    };

    /**
        Join the thread that is executing the async-function.
    */
    virtual ~AsyncLooper()
    {
        m_go = false;
        if ( ThreadWasCreated() )
        {
            pthread_join( m_thread, nullptr );
        }
    };

    bool ThreadWasCreated() const { return m_createError == 0; };

protected:
    virtual void Call() { m_asyncFunction(); };

    static void* Go( void* arg )
    {
        AsyncLooper& async = *reinterpret_cast<AsyncLooper*>( arg );
        async.Call();

        const int sleepTime = 1000000.f / async.m_Hz;
        const std::chrono::microseconds duration( sleepTime );

        using namespace std::chrono;
        while ( async.m_go )
        {
            const high_resolution_clock::time_point nextTime = high_resolution_clock::now() + duration;
            async.Call();
            std::this_thread::sleep_until( nextTime );
        }

        return nullptr;
    };

private:
    Type      m_asyncFunction;
    pthread_t m_thread;
    int       m_createError;
    float     m_Hz;
    volatile bool m_go;
};

#endif /* __ASYNC_LOOPER_H__ */
