cmake_minimum_required ( VERSION 2.8 )

# Try to find videolib

# Once done this will define
#  VIDEOLIB_FOUND - System has Glk
#  VIDEOLIB_INCLUDE_DIRS - The Glk include directories
#  VIDEOLIB_LIBRARIES - The libraries needed to use Glk

# Dependencies - if you get an error about this include you need
# to get a copy of LibFindMacros.cmake (it is not currently included
# with CMake)
include(LibFindMacros)
libfind_package ( VIDEOLIB GLK )

set( INCLUDE_HINT /usr/local/videolib/include )
set( LIB_HINT /usr/local/videolib/lib )

find_path( VIDEOLIB_INCLUDE_DIR
           NAMES videolib.h
           HINTS ${INCLUDE_HINT} )

find_library( VIDEOLIB_LIBRARY
              NAMES glkcore libglkcore
              HINTS "/usr/local/glk/lib" )

# Set the include dir variables and the libraries and let libfind_process do the rest.
# NOTE: Singular variables for this library, plural for libraries this lib depends on.
set( VIDEOLIB_PROCESS_INCLUDES VIDEOLIB_INCLUDE_DIR )
set( VIDEOLIB_PROCESS_LIBS VIDEOLIB_LIBRARY GLK_LIBRARIES )
libfind_process(GLK)

