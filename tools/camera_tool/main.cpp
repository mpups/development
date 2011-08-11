// Copyright (c) 2010 Mark Pupilli, All Rights Reserved.

#include <glkcore.h>
#include <glk.h>
#include "CameraWindow.h"

using namespace GLK;

/**
    Program for controlling robot-motors using lua script console.
**/
int main( int argc, char** argv )
{
    // start console
    CameraWindow win( String("Camera Tool") );

    win.Show();    
    win.StartEventLoop();

    return EXIT_SUCCESS;
}
