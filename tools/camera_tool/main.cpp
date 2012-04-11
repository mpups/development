// Copyright (c) 2010 Mark Pupilli, All Rights Reserved.

#include <glkcore.h>
#include <glk.h>
#include "CameraWindow.h"

using namespace GLK;

/**
    Program for displaying camera feed and also acts as a basic calibration tool.
**/
int main( int argc, char** argv )
{
    CameraWindow win( String("Camera Tool") );
    win.Show();
    win.StartEventLoop();
    return EXIT_SUCCESS;
}
