fileName = 'data.txt'
file = io.open ( fileName , "w" )

t0 = getAmps(1)

max_speed = 3000

for i=0,max_speed,100 do
    t1,a1 = getAmps(1)
    t2,a2 = getAmps(2)
    t = (t1+t2)/2;
    setSpeeds(i,i)
    file:write( t-t0,' ', a1,' ' )  
    file:write( a2,' ', i/counts_per_metre, '\n' )
    sleep(50)
end

for i=0,10 do
    t1,a1 = getAmps(1)
    t2,a2 = getAmps(2)
    t = (t1+t2)/2;
    file:write( t-t0,' ', a1,' ' )  
    file:write( a2,' ', max_speed/counts_per_metre, '\n' )
    sleep(50)
end

for i=max_speed,0,-100 do
    t1,a1 = getAmps(1)
    t2,a2 = getAmps(2)
    t = (t1+t2)/2;
    setSpeeds(i,i)
    file:write( t-t0,' ', a1,' ' )  
    file:write( a2,' ', i/counts_per_metre, '\n' )
    sleep(50)
end

setMoves(0,0)

io.close( file )

os.execute( 'gri ../motor-plot.gri ' .. fileName )

