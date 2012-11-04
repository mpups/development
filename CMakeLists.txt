# May work with earlier CMake version but only tested with the following:
cmake_minimum_required ( VERSION 2.8 )

# GLK Root Description:
project( Video-Lib )

# Good practice to have special debug library suffix:
set( CMAKE_DEBUG_POSTFIX "_d" CACHE STRING "Debug library suffix." ) 

if( NOT WIN32 )
    # These are specifically for Linux builds at the moment:
    set( CMAKE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX}/videolib )
    # Setup RPATH so that installed executables can find installed shared libs:
    SET(CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_PREFIX} )
    SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "/usr/local/glk/share/cmake/Modules/")

find_package (GLK REQUIRED)
if ( GLK_FOUND )
    include_directories( ${GLK_INCLUDE_DIRS} )
    set( LIBS ${LIBS} ${GLK_LIBRARIES})
endif()

add_definitions( -D__STDC_CONSTANT_MACROS -D__STDC_LIMIT_MACROS )

# My custom ffmpeg installation (Ubuntu install is an old broken version):
include_directories( "/home/mark/tmp_installs/include" )
include_directories( "/home/mark/tmp_installs/include" )
set( FFMPEG_LIBRARIES "/home/mark/tmp_installs/lib/libavcodec.so" "/home/mark/tmp_installs/lib/libavformat.so" "/home/mark/tmp_installs/lib/libavutil.so" "/home/mark/tmp_installs/lib/libswscale.so" )

set( LIBS ${LIBS} ${FFMPEG_LIBRARIES})

# Unicap and dc1394:
include_directories( "/usr/include/unicap" )
set( LIBS ${LIBS} /usr/lib/libunicap.so /usr/lib/x86_64-linux-gnu/libdc1394.so )

# Build library:
add_subdirectory( ${PROJECT_SOURCE_DIR}/src/ )

# Build Tests:
add_subdirectory( ${PROJECT_SOURCE_DIR}/src/tests )

# Header File installation:
# Create a list containing all the directories which contain headers to be installed:
set( INSTALL_HEADER_DIRS
    include
    src/io
    src/video
)

# For each item in list, find all .h files, and install them:
foreach( DIR ${INSTALL_HEADER_DIRS} )
    FILE( GLOB INSTALL_HEADERS ${DIR}/*.h )
    install( FILES ${INSTALL_HEADERS} DESTINATION ${DIR} )
endforeach()


