/*
    Copyright (C) Mark Pupilli 2012, All rights reserved
*/
#ifndef __LOAD_BALANCING_CORNER_DETECTOR_H__
#define __LOAD_BALANCING_CORNER_DETECTOR_H__

#include "FastCornerThread.h"

namespace robo
{

/**
    Class which manages multi-threaded fast corner detectors in order to optimise
    throughput of corner detections.

    The FAST corner detector works by quickly rejecting non-corner like features
    to save processing power. This means that it runs faster in a image with few
    corners than in one with many. Hence, a multi-threaded corner detection using
    FAST should not split the work in terms of equal image area, but rather in
    terms of equal numbers of corner detections.

    The key assumption is that the distribution of corners is going to be similar
    between the current frame and the next. Hence a record is kept of how many
    corners were detected in each part of the image on the previous iteration,
    and this is used to split the work for the next iteration.

    @todo the work split should be based on time taken at previous iteration - not
    the number of corners (because the time to reject non-corners is not zero).

    @todo Currently fixed for 2 detection threads - generalise to N threads.
*/
class LoadBalancingCornerDetector
{
public:
    LoadBalancingCornerDetector( uint32_t width, uint32_t height, uint32_t stride, uint8_t* buffer );
    virtual ~LoadBalancingCornerDetector();

    void DisableBalancing();
    void EnableBalancing();

    void Detect( int threshold, std::vector< PixelCoord >& results );

    uint32_t GetSplitHeight() const;

protected:
    void ReBalance();

private:
    uint32_t m_height;
    uint32_t m_splitHeight;

    FastCornerThread m_cornerDetectThread1;
    FastCornerThread::Job m_cornerJob1;
    uint32_t m_cornerCount1;

    uint32_t m_yOffsetJob2;
    FastCornerThread m_cornerDetectThread2;
    FastCornerThread::Job m_cornerJob2;
    uint32_t m_cornerCount2;

    bool m_balancingEnabled;
};

} // end of namespace robo

#endif /* __LOAD_BALANCING_CORNER_DETECTOR_H__ */

