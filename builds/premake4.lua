if ( _ACTION == nil ) then
    dofile ( 'auto_action.lua' ) -- No defualt given so automatically set action based on OS
end

--dofile( 'configure.lua' )
dofile( 'configure_arm.lua' )

-- Append individual link groups together so we can use LINKS to link to all libs:
function append( A, B )
  for i,v in pairs(B) do table.insert(A,v) end
end
append( LINKS, FFMPEG_LINKS )
append( LINKS, SYSTEM_LINKS )
--for i,v in pairs(LINKS) do print(i,v) end

SRC = '../src/'

solution 'videolib'
    configurations { 'debug', 'release', 'profile' }
    platforms { 'gnucross', 'native', }
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

    project 'videolib'
        kind(LIB_TYPE)
        includedirs { "../include" }
        files { SRC .. 'io/**.cpp' }
        files { SRC .. 'video/**.cpp' }

    project 'transcode'
        kind 'ConsoleApp'
        includedirs { "../include" }
        files { SRC .. 'tests/tools/transcode.cpp' }
        configuration {}
        links { 'videolib' }
        links ( GLK_LINKS )
        links ( LINKS )

    project 'gtests' -- unit tests
        kind 'ConsoleApp'

        files { SRC .. 'tests/unit/*.cpp' }
        configuration {}
        links { 'videolib' }
        links ( LINKS )
        links ( 'gtest' )

