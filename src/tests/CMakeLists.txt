find_package (GTest)

if (GTEST_FOUND)
    include_directories( ${GTEST_INCLUDE_DIR} )
    set( LIBS ${LIBS} ${GTEST_LIBRARY_DEBUG} )

    # FindGTest is annoying so fix it:
    if ( NOT ${GTEST_LIBRARY_DEBUG} )
        set( GTEST_LIBRARY_DEBUG ${GTEST_LIBRARY} CACHE FILEPATH "" FORCE )
        message( "WARNING: GTest debug library not found - setting to release library." )
    endif()

    if ( NOT ${GTEST_MAIN_LIBRARY_DEBUG} )
        set( GTEST_MAIN_LIBRARY_DEBUG ${GTEST_MAIN_LIBRARY} CACHE FILEPATH "" FORCE )
        message( "WARNING: GTest-main debug library not found - setting to release library." )
    endif()

elseif()
  set( GTEST_BUILD_TESTS OFF CACHE BOOL )
endif()

project( utest )

add_executable( utest WIN32
    unit/gtests.cpp
    unit/VideoTests.cpp
    unit/VideoTests.h
)
target_link_libraries( utest videolib ${LIBS} )

find_package (GLK REQUIRED)
if ( GLK_FOUND )
    include_directories( ${GLK_INCLUDE_DIRS} )
    set( LIBS ${LIBS} ${GLK_LIBRARIES})
endif()

add_executable( transcode WIN32
    tools/transcode.cpp
)
target_link_libraries( transcode videolib ${LIBS} )

add_executable( capture WIN32
    tools/capture.cpp
)
target_link_libraries( capture videolib ${LIBS} )

install( TARGETS utest DESTINATION bin )

