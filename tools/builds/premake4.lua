if ( _ACTION == nil ) then
    dofile ( '../../builds/auto_action.lua' ) -- No defualt given so automatically set action based on OS
end

-- Beginning of Build Description:

dofile 'configure.lua'

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

    project 'motor-console'
        kind 'ConsoleApp'

        files { '../robo_script/*.cpp' }

        configuration 'debug'
            links { 'glk_d', 'glkcore_d', 'robolib_d' }

        configuration 'release'
            links { 'glk', 'glkcore', 'robolib' }

        configuration { 'profile' }
                links { 'glk_prof', 'glkcore_prof', 'robolib_prof' }

        configuration {}
                links {	LINKS }

