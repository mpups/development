if ( _ACTION == nil ) then
    dofile ( 'auto_action.lua' ) -- No defualt given so automatically set action based on OS
end

--dofile( 'configure.lua' )
dofile( 'configure_arm.lua' )

-- Append individual link groups together so we can use LINKS to link to all libs:
function append( A, B )
  for i,v in pairs(B) do table.insert(A,v) end
end
if ( CONFIGURING_ARM == nil or CONFIGURING_ARM == false ) then
    append( LINKS, OPENCV_LINKS )
    append( LINKS, OPENGL_LINKS )
end
append( LINKS, VIDEO_LINKS )
append( LINKS, GLK_LINKS )
append( LINKS, FFMPEG_LINKS )
append( LINKS, SYSTEM_LINKS )
--for i,v in pairs(LINKS) do print(i,v) end

SRC = '../src/'

solution 'robolib'
    configurations { 'debug', 'release', 'profile' }
    platforms { 'native', 'gnucross' }
    toolchain ( TOOLCHAIN )

    location( TARGET_DIR )
    targetdir( TARGET_DIR )
    language 'C++'

    buildoptions ( BUILD_OPTIONS )
    linkoptions ( LINK_OPTIONS )
    flags { 'NoExceptions','NoRTTI','ExtraWarnings','NoPCH' }

    includedirs { INCLUDE_DIRS }
    libdirs { LIB_DIRS }
    defines ( DEFINES )

    configuration 'release'
        defines { 'NDEBUG' }
        flags { 'OptimizeSpeed','NoFramePointer' }

    configuration 'debug'
        defines { 'DEBUG' }
        flags { 'Symbols' }
        targetsuffix( '_d' )

    configuration 'profile'
        defines { 'NDEBUG' }
        flags { 'Symbols', 'OptimizeSpeed' }
        targetsuffix( '_prof' )
        buildoptions( '-pg -fno-omit-frame-pointer -static-libgcc' )
        linkoptions( '-pg' )
        
    project 'robolib'
        kind(LIB_TYPE)
        files { SRC .. '**.h', SRC .. '**.cpp', SRC .. '**.c' }
        files { SRC .. 'motor_control/*.cpp' }
        excludes { SRC .. 'sse/test/*' }
        excludes { SRC .. 'tests/**' }
        excludes { SRC .. 'sse/*' }
        excludes { SRC .. 'puppybot/*' }
        if ( CONFIGURING_ARM ) then
            excludes { SRC .. 'visualisation/*' }
            excludes { SRC .. 'opencv/*' }
        end

        -- Exclude files which are platform specific:
        if ( PLATFORM == 'win32' ) then
            excludes {
                SRC .. 'io/linux/*'
            }
        elseif ( PLATFORM == 'linux' ) then
            excludes {
                SRC .. 'io/win32/*'
            }
        end

    project 'puppybot-test'
        kind 'ConsoleApp'
        
        files { SRC .. 'puppybot/puppybot_test.cpp' }

        configuration {}
        links { 'robolib' }
        links ( GLK_LINKS )
        links { LINKS }

    project 'puppybot-comms'
        kind 'ConsoleApp'

        files { SRC .. 'puppybot/*.cpp' }
        excludes { SRC .. 'puppybot/puppybot_test.cpp' }

        configuration {}
        links { 'robolib' }
        links ( GLK_LINKS )
        links ( LINKS )

if ( not CONFIGURING_ARM ) then
    project 'sse-test'
        kind 'ConsoleApp'
        files { SRC .. 'sse/**.cpp' }
        links ( GLK_LINKS )
        links ( LINKS )
end

    project 'packet-streaming'
        kind 'ConsoleApp'
        includedirs { "../include" }
        files { SRC .. 'tests/tools/PacketStreaming.cpp' }
        configuration {}
        links { 'robolib' }
        links ( LINKS )

    project 'gtests' -- unit tests
        kind 'ConsoleApp'

        files { SRC .. 'tests/unit/*.cpp' }
        if ( CONFIGURING_ARM ) then
            excludes { SRC .. 'tests/unit/VideoTests*' }
        end
        configuration {}
        links { 'robolib' }
        links ( LINKS )
        links ( 'gtest' )

