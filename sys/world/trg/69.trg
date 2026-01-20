#6904
dance of dreams~
0 k 100
~
wait 1
cas 'dance of dreams' %actor.name%
~
#6929
king diom knight shoot arrow~
0 h 100
~
if %actor.is_npc% != 0 || %actor.level% > 50
halt
else
shoot arrow %actor.name%
end
~
$~
