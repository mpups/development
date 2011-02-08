if ( _ACTION == nil ) then
    dofile ( 'auto_action.lua' ) -- No defualt given so automatically set action based on OS
end

dofile( 'configure.lua' )

SRC = '../src/'

solution 'robolib'
	configurations { 'debug', 'release', 'profile' }
	platforms { 'native', 'gnucross' }
	toolchain ( TOOLCHAIN )
		
	location( TARGET_DIR )
	targetdir( TARGET_DIR )
	language 'C++'
	
    buildoptions ( BUILD_OPTIONS )
    linkoptions ( LINK_OPTIONS)
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
														
		files { SRC .. '**.h', SRC .. '**.cpp' } -- recursivley add all cpp files
		excludes { SRC .. 'sse/test/*' }
        excludes { SRC .. 'tests/*' }

        excludes { SRC .. 'sse/*' }
        excludes { SRC .. 'camera_capture/Dc1394Camera.*' }
		
		-- Then exclude files which are platform specific
		if ( PLATFORM == 'win32' ) then
			excludes {
			    SRC .. 'io/linux/*'
			}
		elseif ( PLATFORM == 'linux' ) then
			excludes {
			    SRC .. 'io/win32/*'
			}
		end

    project 'tests'
        kind 'ConsoleApp'
        
        files { SRC .. 'tests/*.cpp' }
        
        links { 'robolib' }

        configuration { 'debug' }
            links { 'glkcore_d' }
        configuration { 'release' }
            links { 'glkcore' }

        configuration {}
            links { LINKS }
        
