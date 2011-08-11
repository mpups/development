
--wheel/motor constants
wheel_base = 0.275 -- approx. wheel base in metres
counts_per_metre = 6800
gear_ratio = 52
counts_per_rev_in = 12
counts_per_rev = counts_per_rev_in*gear_ratio*4
revs_per_metre = counts_per_metre/counts_per_rev

-- some useful functions:
abs = function(a) if a<0 then return -a else return a end end
sgn = function(a) if a<0 then return -1 else return 1 end end

-- set speed and move commands (units are counts per second):
function setSpeeds( left, right )
    local t1, p1 = setSpeed( 1, -left )
    local t2, p2 = setSpeed( 2, right )
    return t1, p1, t2, p2
end

function setMoves( left, right )
    local t1, p1 = setMove( 1, -left )
    local t2, p2 = setMove( 2, right )
    return t1, p1, t2, p2
end

function setSpeeds_mps( left, right ) -- set wheel speeds in metres per sec
    return setSpeeds( left*counts_per_metre, right*counts_per_metre )
end

function setMotion( v, a ) -- set motion as fwd velocity (v in mps) and angular rate (a in degrees per sec)
    local w = a * wheel_base * (3.141592653/180) -- convert rate to radians per sec and multiply by wheel base
    local vl = v - w
    local vr = v + w
    setSpeeds_mps( vl, vr )
end

function getAmps( addr )
    local time, amps = readReg( addr, 'AMPS' )
    return time, amps*0.02 -- converts ADC value to amps
end

function getPwm( addr )
    local time , pwm = readReg( addr, 'PWMOUT' )
    return time, pwm*(100/1024) -- convert to percentage
end

function getVel( addr )
    return readReg( addr, 'VELOCITY' )
end

function getRev( addr )
    return readReg( addr, 'REVISION' )
end

function getPos( addr )
    return readReg( addr, 'POSITION' )
end

function getVel( addr )
    return readReg( addr, 'VELOCITY' )
end

-- set PID control params :
P = 2500
I = 0
D = 10000
s = 10
ff = 128

-- Write PID and other registers for each motor:

motor_volts   = 7.2
battery_volts = 9.6

for i=1,2 do
    local ok = setupMotor (
        i,
        {
            FUNCTION=82, -- FUNCTION register settings: 2=RETPOS, 66=RETPOS+VELLIMIT, 82=RETPOS+VELLIMIT+SATPROT 
            PTERM=P, ITERM=I, DTERM=D, PIDSCALAR=s, VELOCITYFF=ff,
            AMPSLIMIT=(1.5/0.02), -- 1.5 Amp limit (approx.) for motors.
            PWMLIMIT=(motor_volts/battery_volts)*1023 -- 1023 is 100% duty cycle
        }
    )
    if (ok) then print "setupMotor() - OK" else print("setupMotor() - FAILED") end
end

