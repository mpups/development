/*
    Copyright (C) Mark Pupilli 2013, All rights reserved
*/
#ifndef __SIMPLE_ASYNC_FUNCTION_H__
#define __SIMPLE_ASYNC_FUNCTION_H__

#include <functional>

#include <pthread.h>

/**
    Can be used to run a std::function in a separate thread.

    The function is moved into the SimpleAsyncFunction on construction
    and no further access is provided to it. A new thread is spawned
    immediately and the function called in that thread's context.

    There is no way to kill the thread prematurely - the function must
    exit of its own accord.

    The internally spawned thread is not joined until the destructor is
    called. The call to the destructor will block until the thread exits
    (function returns).

    Example - execute a lambda in the context of a new thread:
\code
        SimpleAsyncFunction( []() {
            // do asynchronous stuff
        });
\endcode

    A SimpleAsyncFunction object can be moved, but not copied.
*/
class SimpleAsyncFunction
{
public:
    typedef std::function< void(void) > Type;

    SimpleAsyncFunction( const SimpleAsyncFunction& ) = delete;
    SimpleAsyncFunction( SimpleAsyncFunction&& ) = delete;

    /**
        Move in a function object and start a thread that will call this function in a separate thread.
    */
    SimpleAsyncFunction( Type&& f )
    :
        m_asyncFunction ( std::move(f) ),
        m_createError   ( pthread_create( &m_thread, nullptr, SimpleAsyncFunction::Go, this ) )
    {
    };

    /**
        Join the thread that is executing the async-function.
    */
    virtual ~SimpleAsyncFunction()
    {
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
        SimpleAsyncFunction& async = *reinterpret_cast<SimpleAsyncFunction*>( arg );
        async.Call();
        return nullptr;
    };

private:
    Type m_asyncFunction;
    pthread_t m_thread;
    int m_createError;
};

#endif /* __SIMPLE_ASYNC_FUNCTION_H__ */
