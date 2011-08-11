
-- setup motors:
dofile '../init-motors.lua'

-- first flush button events
while getJoyButtonEvent() ~= nil do end

-- open file for logging current data
fileName = 'motor-current.txt'
file = io.open ( fileName , "w" )
local t0 = getAmps(1)

print( 'joy-control started...' )
local max_speed = 1.0 -- metres per second

local ral = 0.0
local rar = 0.0
local w   = 0.4

local go = true
while go do

    sleep( 100 ) -- defines update rate

    local jx, jy = getJoyAxes() -- jx and jy range from -32767 to 32767

    -- apply deadbands
    if abs(jx) < 3000 then jx = 0 end
    if abs(jy) < 3000 then jy = 0 end

    local v = ( -jy / 32767 ) * max_speed -- forward speed in range (-max_speed,max_speed) m/sec
    local tr = ((jx/32767) * 0.5) -- turn rate
    local vl = v + tr
    local vr = v - tr

    -- apply min speed limit (otherwise motors will judder)
    if ( abs(vl) < 0.075 ) then vl = 0 end
    if ( abs(vr) < 0.075 ) then vr = 0 end

    --print( 'Sending speeds:', vl, vr )
    local tl, pl, tr, pr = setSpeeds_mps( vl, vr )-- sets left and right speeds in metres per second
    --print( pl, pr )

    -- first log current data:
    local tl,al = getAmps(1)
    local tr,ar = getAmps(2)
    local t = (tl+tr) / 2;
    ral = (1-w)*ral + (w*al);
    rar = (1-w)*rar + (w*ar);

    -- DATA: time, amps-left, running-avg-left, amps-right, running-avg-right, speed-left, speed-right
    file:write( t-t0,' ', al,' ',ral,' ',abs(vl),' ',ar,' ', rar, ' ', abs(vr), '\n' ) 

    -- run through all button events
    local button, val
    repeat
        button, val = getJoyButtonEvent()
        if button == 16 and val == 1 then go = false end -- button 16 quits!
    until button == nil
end

setMoves(0,0) -- minimise current draw
print( '...joy-control finished' )

-- write current data and plot it
io.close( file )
os.execute( 'gri ../motor-plot.gri ' .. fileName )

