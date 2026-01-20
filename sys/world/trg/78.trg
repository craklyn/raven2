#7800
pax test trigger 1~
1 c 2
goto~
if %actor.is_npc% || %arg.strlen% == 0
    %send% %actor% Huh?!?
    halt
end
%teleport% %actor% %arg%
%force% %actor% look
~
#7801
pax test trigger 2~
1 c 2
ocast~
if %actor.is_npc% || %arg.strlen% == 0
    %send% %actor% Huh?!?
    halt
end
eval targ %arg.car%
eval spell %arg.cdr%
dg_cast '%spell%' %targ%
~
#7802
pax test trigger 3~
1 c 2
buffs~
if %actor.is_npc% || %arg.strlen% == 0
    %send% %actor% Huh?!?
    halt
end
dg_cast '%arg%' %actor.name%
~
#7803
pax test trigger 4~
1 c 2
oexp~
if %actor.is_npc% || %arg.strlen% == 0
    %send% %actor% Huh?!?
    halt
end
oexp %actor.name% %arg%
if %arg% < 1000000
    %send% %actor% You gained %arg% experience!
elseif %arg% >= 1000000
    %send% %actor% You gained 1000000 experience!
else
    %send% %actor% Huh?!?
end
~
#7804
pax oload test~
1 c 2
oload~
if %actor.is_npc% || %arg.strlen% == 0
    %send% %actor% Huh?!?
    halt
end
%load% obj %arg%
%force% %actor% get 1.
~
#7805
pax oload test~
1 c 2
mload~
if %actor.is_npc% || %arg.strlen% == 0
    %send% %actor% Huh?!?
    halt
end
%load% mob %arg%
eval in_room %actor.room%
eval mob %in_room.people%
%send% %actor% You created %mob.name%!
~
$~
