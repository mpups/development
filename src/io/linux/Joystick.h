#ifndef ROBO_JOYSTICK_H
#define ROBO_JOYSTICK_H

#include <cstdint>
#include <thread>
#include <mutex>
#include <queue>

typedef const char* JoystickDevice_t;
typedef int JoystickHandle_t;

/**
    Class for basic joystick access.

    The class derives from thread and when started the thread
    runs a task which receives and processes joystick events.
    The thread does not run immediately - you must call Start()
    on the joystick object. However, the thread is cleanly shutdown
    on destruction.

    Currently Linux only.
**/
class Joystick
{
public:
    struct ButtonEvent
    {
        uint8_t  id;
        uint16_t pressed; // 1 if pressed down, 0 it released
    };

    Joystick( JoystickDevice_t device );
    ~Joystick();

    bool IsAvailable();
    int16_t GetAxis( unsigned int i )   const { return m_axis[i]; };
    int16_t GetButton( unsigned int i ) const { return m_button[i]; };
    bool GetButtonEvent( ButtonEvent& e );

    void Run();

private:
    static const int m_POLL_TIMEOUT_MS = 600; // timeout in millisecs

    JoystickHandle_t m_joy;
    int16_t* m_button;
    int16_t* m_axis;

    std::mutex m_queueLock;
    std::queue<ButtonEvent> m_buttonEvents;

    volatile bool m_terminate;
    std::thread   m_thread;
};

#endif // ROBO_JOYSTICK_H

