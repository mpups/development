#ifndef TIMER_H
#define TIMER_H

#include <time.h>
#include <cstdint>

class Timer
{
public:
    Timer(clockid_t clock) : m_clockToUse(clock) { Reset(); }

    void Reset() { clock_gettime( m_clockToUse, &m_time ); }
    double GetSeconds() const;
    std::uint32_t GetMilliSeconds() const {return (GetSeconds()*1000.0);}

private:
    static double TimespecToSeconds(const timespec);

    const clockid_t m_clockToUse;
    timespec m_time;
};

inline double Timer::GetSeconds() const
{
    timespec current;
    clock_gettime( m_clockToUse, &current );
    return TimespecToSeconds(current) - TimespecToSeconds(m_time);
}

inline double Timer::TimespecToSeconds(const timespec t)
{
    return t.tv_sec + (t.tv_nsec*0.000000001);
}

#endif // TIMER_H
