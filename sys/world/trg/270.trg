#27001
Randomize East~
2 c 100
e~
%echo% %actor.name% became lost in the fog!
eval rand %random.4%
if %rand% == 1
  %force% %actor% north
elseif %rand% == 2
  %force% %actor% east
elseif %rand% == 3
  %force% %actor% south
else
  %force% %actor% west
end
return 0
~
#27002
Randomize South~
2 c 100
so~
%echo% %actor.name% became lost in the fog!
eval rand %random.4%
if %rand% == 1
%force% %actor% north
elseif %rand% == 2
%force% %actor% east
elseif %rand% == 3
%force% %actor% south
else
%force% %actor% west
end
return 0
~
#27003
Randomize West~
2 c 100
w~
%echo% %actor.name% became lost in the fog!
eval rand %random.4%
if %rand% == 1
%force% %actor% north
elseif %rand% == 2
%force% %actor% east
elseif %rand% == 3
%force% %actor% south
else
%force% %actor% west
end
return 0
~
#27004
Randomize N~
2 c 100
n~
%echo% %actor.name% became lost in the fog!
eval rand %random.4%
if %rand% == 1
%force% %actor% north
elseif %rand% == 2
%force% %actor% east
elseif %rand% == 3
%force% %actor% south
else
%force% %actor% west
end
return 0
~
#27005
GreetAttack~
0 g 80
~
if (%actor.is_npc% != 0 || %actor.level% > 50)
halt
else
kill %actor.name%
end
~
$~
