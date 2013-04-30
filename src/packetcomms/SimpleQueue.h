/*
    Copyright (C) Mark Pupilli 2013, All rights reserved
*/
#ifndef __SIMPLE_QUEUE_H__
#define __SIMPLE_QUEUE_H__

#include <queue>

/// @todo - sort this out:
#include "/home/mark/code/glk/src/glkcore/thread/MutexLock.h"
#include "/home/mark/code/glk/src/glkcore/thread/posix/Mutex.h"
#include "/home/mark/code/glk/src/glkcore/thread/posix/ConditionVariable.h"

#include <assert.h>
#include "../packetcomms/ComPacket.h"

/**
    Simple FIFO message queue for com-packets.
*/
class SimpleQueue
{
public:
    /**
        This allows you to hold a lock on a queue and have that
        lock automatically released when the LockedQueue object
        goes out of scope.

        A LockedQueue can only be constructed by a call to SimpleQueue::Lock();
    */
    class LockedQueue
    {
    friend class SimpleQueue;

    public:
        LockedQueue( const LockedQueue& ) = delete;
        LockedQueue( LockedQueue&& other ) : m_secureLock(other.m_secureLock) { other.m_secureLock = nullptr; };
        virtual ~LockedQueue() { if ( m_secureLock != nullptr ) { m_secureLock->Unlock(); } };
    private:
        LockedQueue( const SimpleQueue& q ) : m_secureLock( &q.m_lock ) { m_secureLock->Lock(); };
        GLK::Mutex* m_secureLock;
    };

    SimpleQueue() {};
    virtual ~SimpleQueue() {};

    /**
        This call will block forever if you hold a LockedQueue from this queue.
    */
    void Emplace( const ComPacket::ConstSharedPacket& item )
    {
        GLK::MutexLock locker( m_lock );
        m_items.emplace( item );
        m_notEmpty.WakeAll();
    };

    /**
        You should acquire the lock before calling any of the functions other than Emplace().
        WaitNotEmpty() explicitly enforces this by requiring you to pass back the lock.

        This call will block forever if you already hold a LockedQueue from this queue.
    */
    LockedQueue Lock() { return LockedQueue( *this ); };

    void WaitNotEmpty( LockedQueue& lock )
    {
        // Check the caller holds the lock on this queue:
        if ( lock.m_secureLock == &m_lock )
        {
            while ( m_items.empty() )
            {
                m_notEmpty.Wait( m_lock );
            }
        }
        else
        {
            assert(false);
        }
    };

    void WaitNotEmpty( LockedQueue& lock, int timeoutInMilliseconds )
    {
        // Check the caller holds the lock on this queue:
        if ( lock.m_secureLock == &m_lock )
        {
            if ( m_items.empty() )
            {
                m_notEmpty.TimedWait( m_lock, timeoutInMilliseconds );
            }
        }
        else
        {
            assert(false);
        }
    };

    // Yo umay or may not get away with calling these without holding a
    // LockedQueue object, but technically you should hold one before calling them:
    size_t Size() const { return m_items.size(); };
    bool Empty() const { return m_items.empty(); };
    const ComPacket::ConstSharedPacket& Front() const { return m_items.front(); };
    void Pop() { m_items.pop(); };

protected:

private:
    mutable GLK::Mutex     m_lock;
    GLK::ConditionVariable m_notEmpty;
    std::queue< ComPacket::ConstSharedPacket > m_items;
};

#endif /* __SIMPLE_QUEUE_H__ */

