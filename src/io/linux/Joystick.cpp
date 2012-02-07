#include "Joystick.h"

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
//#include <sys/time.h>
#include <poll.h>
#include <linux/joystick.h>
#include <assert.h>

Joystick::Joystick( JoystickDevice_t device )
:
    m_button        (0),
    m_axis          (0),
    m_buttonEvents  (32),
    m_terminate     (false)
{
    m_joy = open( device, O_RDONLY );
    fprintf( stderr,"Opened %s: %d\n", device, m_joy );

    if ( m_joy )
    {
        m_button = new int16_t[256];
        m_axis   = new int16_t[256];
        
        // Zero all buttons and axes:
        memset( m_button, 0, 256*sizeof(int16_t) );
        memset( m_axis, 0, 256*sizeof(int16_t) );
    }
}

/**
    Terminate thread and then shutdown cleanly.
*/
Joystick::~Joystick()
{
    if ( m_joy > 0 )
    {
        m_terminate = true;
        Thread::Join();

        close( m_joy );
        delete m_button;
        delete m_axis;
    }
}

/**
    @return true if joystick device is available, false otherwise
**/
bool Joystick::IsAvailable()
{
    return m_joy > 0;
}

/**
    Thread which loops while polling for joystick events.

    Uses non-blocking IO and select() so that thread can be cleanly terminated.
**/
void Joystick::Run()
{
    if ( !IsAvailable() )
    {
        fprintf( stderr, "WARNING: Started Joystick thread but there is no joystick available\n" );
        assert(0);
        return;
    }

    assert( m_joy > 0 );
    
    // Switch to non-blocking I/O before we enter the event loop:
    fcntl( m_joy, F_SETFL, O_NONBLOCK );

    struct pollfd pfds;
    pfds.fd = m_joy;
    pfds.events = POLLIN;
    pfds.revents = 0;
    struct js_event e;
    ButtonEvent button;
    while ( m_terminate == false )
    {
        int val = poll( &pfds, 1, m_POLL_TIMEOUT_MS );
        assert( val >= 0 );
        if ( val == 1 && pfds.revents & POLLIN )
        {   
            val = read ( m_joy, &e, sizeof(struct js_event) );
            if ( val == sizeof(struct js_event) )
            {
                switch ( e.type & ~JS_EVENT_INIT )
		        {
		            case JS_EVENT_AXIS:
                        m_axis[e.number] = e.value;
        	    	break;
			    
                    case JS_EVENT_BUTTON:
                        m_button[e.number] = e.value;

                        // For non-init events we put a button event in the message queue:
                        if ( !(e.type & JS_EVENT_INIT) )
                        {
                            button.id = e.number;
                            button.pressed = e.value;
                            m_buttonEvents.TryWrite( &button );
                        }
                    break;

                    default:
                    break;
    		    }
            }

        }        
    }
    fcntl( m_joy, F_SETFL, O_SYNC ); // blocking I/O back on
}

/**
    Test and get if there is a button event in this joystick's button message queue.
    @param e Storage for the received event.
    @return true if an event was received, false if the queue is empty.
*/
bool Joystick::GetButtonEvent( ButtonEvent& e )
{
    return m_buttonEvents.TryRead( &e );
}

