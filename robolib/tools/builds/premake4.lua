if ( _ACTION == nil ) then
    dofile ( '../../builds/auto_action.lua' ) -- No defualt given so automatically set action based on OS
end

dofile( 'configure.lua' )

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

solution 'tools'
    configurations { 'debug', 'release', 'profile' }
    location( TARGET_DIR )
    targetdir( TARGET_DIR )
    language 'C++'
    flags { 'ExtraWarnings' }

    includedirs ( INCLUDE_DIRS )
    libdirs( LIB_DIRS )
    buildoptions ( BUILD_OPTIONS )
    linkoptions ( LINK_OPTIONS )
    defines { DEFINES }

    configuration 'debug'
        defines { 'DEBUG' }
        flags { 'Symbols' }
        targetsuffix( '_d' )

    configuration 'release'
        defines { 'NDEBUG' }
        flags { 'OptimizeSpeed','NoFramePointer' }

    configuration 'profile'
        defines { 'NDEBUG' }
        flags { 'OptimizeSpeed','Symbols' }
        targetsuffix( '_prof' )
        buildoptions { "-pg -fno-omit-frame-pointer -static-libgcc" }
        linkoptions { "-pg" }

    project 'camera-view'
        kind 'ConsoleApp'

        files { '../camera_tool/*.cpp' }

        configuration 'debug'
                links { 'robolib_d', 'glk_d', 'glkcore_d' }
        configuration { 'release' }
                links { 'robolib', 'glk', 'glkcore' }
        configuration { 'profile' }
                links { 'robolib_prof', 'glk_prof', 'glkcore_prof' }

        configuration {}
                links { LINKS }

    project 'image-proc-lab'
        kind 'ConsoleApp'

        files { '../image-proc-lab/*.cpp' }

        configuration 'debug'
                links { 'robolib_d', 'glk_d', 'glkcore_d' }
        configuration { 'release' }
                links { 'robolib', 'glk', 'glkcore' }
        configuration { 'profile' }
                links { 'robolib_prof', 'glk_prof', 'glkcore_prof' }

        configuration {}
                links { LINKS }
