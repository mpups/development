if ( _ACTION == nil ) then
    dofile ( 'auto_action.lua' ) -- No defualt given so automatically set action based on OS
end

dofile( 'configure.lua' )

SRC = '../'

solution 'robolib'
	configurations { 'debug', 'release', 'profile' }
	location( TARGET_DIR )
	targetdir( TARGET_DIR )
	language 'C++'
	flags { 'NoExceptions','NoRTTI','ExtraWarnings','NoPCH', 'EnableSSE', 'EnableSSE2' }
	
    includedirs { LUA_INC, GLK_INC, FREETYPE2_INC, UNICAP_INC, DC1394_INC }
    defines ( EXTRA_DEFINES )
    buildoptions ( EXTRA_BUILD_OPTIONS )
    
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

