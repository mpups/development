/**
    Copyright (c) mark Pupilli 2011
**/

#include <gtest/gtest.h>

/**
    Runs all the tests listed above.
**/
int main( int argc, char** argv )
{
    ::testing::InitGoogleTest( &argc, argv );
    int v = RUN_ALL_TESTS();
    return v;
}

