-- Auto detect the operating system and set a default action accordingly.

if ( os.is( 'linux' ) ) then
    PLATFORM = 'linux'
    print( 'Auto-action: Linux detected - using gmake' )
    _ACTION = 'gmake'
elseif ( os.is( 'windows' ) ) then
    PLATFORM = 'win32'
    print( 'Auto-action: Windows detected - using vs2008' )
    _ACTION = 'vs2008'
end
