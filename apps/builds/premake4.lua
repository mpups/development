if ( _ACTION == nil ) then
    dofile ( '../../builds/auto_action.lua' ) -- No defualt given so automatically set action based on OS
end

dofile( 'configure.lua' )

-- Append individual link groups together so we can use LINKS to link to all libs:
function append( A, B )
  for i,v in pairs(B) do table.insert(A,v) end
end
append( LINKS, OPENCV_LINKS )
append( LINKS, OPENGL_LINKS )
append( LINKS, SYSTEM_LINKS )
--for i,v in pairs(LINKS) do print(i,v) end

SRC = '../src/'

solution 'apps'
    configurations { 'debug', 'release', 'profile' }
    platforms { 'native', 'gnucross' }
    toolchain ( TOOLCHAIN )

    location( TARGET_DIR )
    targetdir( TARGET_DIR )
    language 'C++'

    buildoptions ( BUILD_OPTIONS )
    linkoptions ( LINK_OPTIONS )
    linkoptions { '-Wl,-rpath,/usr/local/lib/glk/' }
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

    project 'puppybot-server'
        kind 'ConsoleApp'

        files { SRC .. 'puppybot/server.cpp', SRC .. 'puppybot/PuppybotServer.cpp' }

        configuration {}
        links { 'robolib' }
        links ( GLK_LINKS )
        links { LINKS }

if ( not CONFIGURING_ARM ) then
    project 'puppybot-client'
        kind 'ConsoleApp'

        files { SRC .. 'puppybot/client.cpp' }

        configuration {}
        links { 'robolib' }
        links ( GLK_LINKS )
        links { LINKS }
end

