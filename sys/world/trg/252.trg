#25201
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
#25250
Dispater Death Crack~
0 f 100
~
%door% 25389 down room 25439
%door% 25439 up room 25389
%echo% You feel a shiver as the Tower of Dis shifts on its foundations.
~
$~
