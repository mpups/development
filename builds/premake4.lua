if ( _ACTION == nil ) then
    dofile ( 'auto_action.lua' ) -- No defualt given so automatically set action based on OS
end

dofile( 'configure.lua' )
--dofile( 'configure_arm.lua' )

-- Append individual link groups together so we can use LINKS to link to all libs:
function append( A, B )
  for i,v in pairs(B) do table.insert(A,v) end
end
append( LINKS, OPENCV_LINKS )
append( LINKS, OPENGL_LINKS )
append( LINKS, SYSTEM_LINKS )
append( LINKS, FFMPEG_LINKS )
append( LINKS, VIDEO_LINKS )
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
        kind 'StaticLib'
        files { SRC .. '**.h', SRC .. '**.cpp', SRC .. '**.c' }
        excludes { SRC .. 'sse/test/*' }
        excludes { SRC .. 'tests/**' }
        excludes { SRC .. 'sse/*' }
        if ( CONFIGURING_ARM ) then
            excludes { SRC .. 'video/Dc1394Camera.*' }
            excludes { SRC .. 'visualisation/*' }
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
        
        files { SRC .. 'puppybot/RobotServer.cpp' }
        files { SRC .. 'puppybot/puppybot_comms.cpp' }
        
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

    project 'transcode'
        kind 'ConsoleApp'
        includedirs { "../include" }
        files { SRC .. 'tests/tools/transcode.cpp' }
        configuration {}
        links { 'robolib' }
        links ( GLK_LINKS )
        links ( LINKS )

    project 'video-streaming'
        kind 'ConsoleApp'
        includedirs { "../include" }
        files { SRC .. 'tests/tools/VideoStreaming.cpp' }
        configuration {}
        links { 'robolib' }
        links ( GLK_LINKS )
        links ( LINKS )

    project 'packet-streaming'
        kind 'ConsoleApp'
        includedirs { "../include" }
        files { SRC .. 'tests/tools/PacketStreaming.cpp' }
        configuration {}
        links { 'robolib' }
        links ( GLK_LINKS )
        links ( LINKS )

    project 'gtests' -- unit tests
        kind 'ConsoleApp'

        files { SRC .. 'tests/unit/*.cpp' }
        if ( CONFIGURING_ARM ) then
            excludes { SRC .. 'tests/unit/VideoTests*' }
        end
        configuration {}
        links { 'robolib' }
        links ( SYSTEM_LINKS )
        links ( FFMPEG_LINKS )
        links ( 'gtest' )

