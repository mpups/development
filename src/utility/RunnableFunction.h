/*
    Copyright (C) Mark Pupilli 2012, All rights reserved
*/
#ifndef __RUNNABLE_FUNCTION_H__
#define __RUNNABLE_FUNCTION_H__

#include <glkcore.h>

#include <functional>

/**
    This allows us to create a Runnable directly from a std::function.
    In turn this means the std::function can be executed asynchronously
    by a GLK::Thread.

    @note The function is stored by value, but RunnableFunction provides a
    constructor which takes an rvalue reference that can be used to avoid
    copying.
*/
class RunnableFunction : public GLK::Runnable
{
public:
    typedef std::function< void(void) > Type;

    RunnableFunction( Type& f ) : m_asyncFunction(f) {}; //< Construct by copying the function.
    RunnableFunction( Type&& f ) : m_asyncFunction( std::move(f) ) {}; //< Construct by moving the function.
    virtual void Run() { m_asyncFunction(); }; //override;
    virtual ~RunnableFunction() {};

private:
    Type m_asyncFunction;
};

#endif /* __RUNNABLE_FUNCTION_H__ */

