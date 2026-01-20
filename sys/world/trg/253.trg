#25301
Hamatula march~
0 b 100
~
if %self.eq(hold)%
        halt
end 
eval room %self.room%
eval dir %room% - ((%room% / 7) * 7)
if (%room% >= 25203) && (%room% <= 25208)
    east
elseif (%room% >= 25246) && (%room% <= 25251)
    west
elseif (%dir% == 2)
    south
else
    north
end 
~
$~
