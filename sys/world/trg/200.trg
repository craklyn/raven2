#20000
The Gold Banner Take Zapper~
1 g 100
~
*Gold Banner take Zapper
if %actor.team% == black
  %send% %actor% The Gold Battle Standard zaps you with a jolt of electricty as you grap it.
  %echoaround% %actor% The Gold Battle Standard emits a shower of sparks as it zaps %actor.name%
  %damage% %actor% 25
end
~
#20001
Maze Randomizer for room 20082~
2 f 100
~
* Maze Randomizer for room 20082
switch %random.4%
 case 1
   * Purge all the exits first
   wdoor %self.vnum% east room 20083^M
   wdoor %self.vnum% north room 20088^M
   wdoor %self.vnum% south room 20088^M
   wdoor %self.vnum% west room 20088^M
   break
  case 2
   wdoor %self.vnum% east room 20088^M
   wdoor %self.vnum% north room 20083^M
   wdoor %self.vnum% south room 20088^M
   wdoor %self.vnum% west room 20088^M
   break
  case 3
   wdoor %self.vnum% east room 20088^M
   wdoor %self.vnum% north room 20088^M
   wdoor %self.vnum% south room 20083^M
   wdoor %self.vnum% west room 20088^M
   break
  case 4
   wdoor %self.vnum% east room 20088^M
   wdoor %self.vnum% north room 20088^M
   wdoor %self.vnum% south room 20088^M
   wdoor %self.vnum% west room 20083^M
   break
~
#20002
Maze Randomizer for room 20081~
2 f 100
~
* Maze Randomizer for room 20081
switch %random.4%
 case 1
   * Purge all the exits first
   wdoor %self.vnum% east room 20082^M
   wdoor %self.vnum% north room 20088^M
   wdoor %self.vnum% south room 20088^M
   wdoor %self.vnum% west room 20088^M
   break
  case 2
   wdoor %self.vnum% east room 20088^M
   wdoor %self.vnum% north room 20082^M
   wdoor %self.vnum% south room 20088^M
   wdoor %self.vnum% west room 20088^M
   break
  case 3
   wdoor %self.vnum% east room 20088^M
   wdoor %self.vnum% north room 20088^M
   wdoor %self.vnum% south room 20082^M
   wdoor %self.vnum% west room 20088^M
   break
  case 4
   wdoor %self.vnum% east room 20088^M
   wdoor %self.vnum% north room 20088^M
   wdoor %self.vnum% south room 20088^M
   wdoor %self.vnum% west room 20082^M
   break
~
#20003
Flag Standard get script (o20001)~
1 g 100
~
*Gold Banner Take Zapper
* Get script for o20001
if %actor.team% == gold && %actor.room% == 20095
   %send% %actor% Your Flag has already been Returned.
   return 0
end
if %actor.team% == rogue && %actor.room% == 20095
   %send% %actor% The Flag has already been returned.
   return 0
end
if %actor.team% == black && %actor.room% == 20095
  %send% %actor% The Gold Battle Standard zaps you with a jolt of electricty as you grap it.
  %echoaround% %actor% The Gold Battle Standard emits a shower of sparks as it zaps %actor.name%
  %damage% %actor% 5
end
~
#20004
Fortress Arrow Storm (Gold)~
2 b 70
~
if %self.people%
   eval victim %random.char%
   if %victim.level% > 50 || %victim.team% == gold || %victim.is_npc% != 0
    halt
   end  
   %echo% A storm of arrows from the &10Gold&00 Fortress sweep across the ground.
   %damage% %victim% %random.100%
   %echoaround% %victim% Several arrows sprout from %victim.name%'s chest.
   %send% %victim% You take several arrows in the chest.
else
  halt
~
#20005
portal closer~
2 b 30
~
switch %random.3%
case 1
%door% 20001 west flags a
%door% 20101 east flags a
break
case 2
%door% 20001 west flags bcd
%door% 20101 east flags bcd
break
case 3
%door% 20001 west flags a
%door% 20101 east flags a
break
done
~
#20006
gauntletrandomizer~
2 b 50
~
switch %random.3%
case 1
%door% 20078 down flags a
%door% 20431 up flags a
%echo% The gates to the Gauntlet slowly begin to rise.
break
case 2
%door% 20078 down flags bcd
%door% 20431 up flags bcd
%echo% The gates to the Gauntlet slam shut with a bang.
break
case 3
%door% 20078 down flags bcd
%door% 20431 up flags bcd
%echo% The gates to the Gauntlet slam shut with a bang.
break
done
~
#20007
gauntrandom2~
2 b 50
~
switch %random.3%
case 1
%door% 20178 down flags a
%door% 20401 up flags a
%echo% The gates to the Gauntlet slowly begin to rise.
break
case 2
%door% 20178 down flags bcd
%door% 20401 up flags bcd
%echo% The gates to the Gauntlet slam shut with a bang.
break
case 3
%door% 20178 down flags bcd
%door% 20401 up flags bcd
break
%echo% The gates to the Gauntlet slam shut with a bang.
done
~
#20008
UW room random~
2 b 50
~
 switch %random.3%
case 1
%door% 20002 down flags a
%door% 24801 up flags a
%echo% The flood waters recede, revealing an entry way below.
break
case 2
%door% 20002 down flags bcd
%door% 24801 up flags bcd
%echo% The flood waters rise, concealing the entry way below.
break
case 3
%door% 20002 down flags bcd
%door% 24801 up flags bcd
%echo% The flood waters rise, concealing the entry way below.
break
done
~
#20009
UW randomize door2~
2 b 50
~
switch %random.3%
case 1
%door% 20102 down flags a
%door% 24811 up flags a
%echo% The flood waters recede, revealing an entry way below.
break
case 2
%door% 20102 down flags bcd
%door% 24811 up flags bcd
%echo% The flood waters rise, concealing the entry way below.
break
case 3
%door% 20102 down flags bcd
%door% 24811 up flags bcd
%echo% The flood waters rise, concealing the entry way below.
break
done
~
#20020
super aggr attack~
0 h 100
~
if %actor.is_npc% != 0 || %actor.level% > 50
halt
else
murder %actor.name%
end
~
#20097
Player Entrance Announcements~
2 g 100
~
eval team %actor.team%
if %team% == gold
return 0
%echo% %actor.name% (%team%) is Re-entering the fray.
%teleport% %actor% 20198
%force% %actor% look
elseif %team% == black
return 0
%echo% %actor.name% %team% is Re-entering the fray.
%teleport% %actor% 20100
%force% %actor% look
elseif %team% == rogue
return 0
%echo% %actor.name% %team% is Re-entering the fray.
%teleport% %actor% 20098
%force% %actor% look
else
%echo% New Player %actor.name% Needs a Team
end
~
$~
