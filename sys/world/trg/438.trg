#43801
Fantr superaggro trigger~
0 h 100
~
if %actor.is_npc% != 0 || %actor.level% > 50
halt
else
back %actor.name%
end
~
#43806
jarai super aggro thief trigger~
0 h 100
~
if %actor.is_npc% != 0 || %actor.level% > 50 
halt
else 
back %actor.name%
feign
stand
back %actor.name%
feign
stand
halt
back %actor.name%
feign
stand
halt
back %actor.name%
~
#43845
citera aggressive trigger~
0 g 100
~
if %actor.is_npc% != 0 || %actor.level% > 50
halt
else
kill %actor.name%
end
~
#43899
elrus trigger~
0 g 100
~
say hello
~
$~
