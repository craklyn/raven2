#20100
The Black Banner Take Zapper~
1 g 100
~
*Black Banner Take Zapper
if %actor.team% == gold
  %send% %actor% The Gold Battle Standard zaps you with a jolt of electricty as you grap it.
  %echoaround% %actor% The Gold Battle Standard emits a shower of sparks as it zaps %actor.name%
  %damage% %actor% 25
end
~
#20101
Maze Randomizer for room 20182~
2 f 100
~
* Maze Randomizer for room 20182
switch %random.4%
 case 1
   * Purge all the exits first
   wdoor %self.vnum% west room 20183^M
   wdoor %self.vnum% north room 20188^M
   wdoor %self.vnum% south room 20188^M
   wdoor %self.vnum% east room 20188^M
   break
  case 2
   wdoor %self.vnum% west room 20188^M
   wdoor %self.vnum% north room 20183^M
   wdoor %self.vnum% south room 20188^M
   wdoor %self.vnum% east room 20188^M
   break
  case 3
   wdoor %self.vnum% west room 20188^M
   wdoor %self.vnum% north room 20188^M
   wdoor %self.vnum% south room 20183^M
   wdoor %self.vnum% east room 20188^M
   break
  case 4
   wdoor %self.vnum% west room 20188^M
   wdoor %self.vnum% north room 20188^M
   wdoor %self.vnum% south room 20188^M
   wdoor %self.vnum% east room 20183^M
   break
~
#20102
Maze Randomizer for room 20181~
2 f 100
~
* Maze Randomizer for room 20181
switch %random.4%
 case 1
   * Purge all the exits first
   wdoor %self.vnum% west room 20182^M
   wdoor %self.vnum% north room 20188^M
   wdoor %self.vnum% south room 20188^M
   wdoor %self.vnum% east room 20188^M
   break
  case 2
   wdoor %self.vnum% west room 20188^M
   wdoor %self.vnum% north room 20182^M
   wdoor %self.vnum% south room 20188^M
   wdoor %self.vnum% east room 20188^M
   break
  case 3
   wdoor %self.vnum% west room 20188^M
   wdoor %self.vnum% north room 20188^M
   wdoor %self.vnum% south room 20182^M
   wdoor %self.vnum% east room 20188^M
   break
  case 4
   wdoor %self.vnum% west room 20188^M
   wdoor %self.vnum% north room 20188^M
   wdoor %self.vnum% south room 20188^M
   wdoor %self.vnum% east room 20182^M
   break
~
#20103
Flag Standard get script (o20101)~
1 g 100
~
*Black Banner Take Zapper
* Get script for o20101
if %actor.team% == black && %actor.room% == 20195
   %send% %actor% Your Flag has already been Returned.
   return 0
end
if %actor.team% == rogue && %actor.room% == 20195
   %send% %actor% The Flag has already been returned.
   return 0
end
if %actor.team% == gold && %actor.room% == 20195
  %send% %actor% The Black Battle Standard zaps you with a jolt of electricty as you grap it.
  %echoaround% %actor% The Black Battle Standard emits a shower of sparks as it zaps %actor.name%
  %damage% %actor% 5
end
~
#20104
Fortress Arrow Storm (black)~
2 b 70
~
if %self.people%
   eval victim %random.char%
   if %victim.level% > 50 || %victim.team% == black || %victim.is_npc% != 0
    halt
   end  
   %echo% A storm of arrows from the &07Black&00 Fortress sweep across the ground.
   %damage% %victim% %random.100%
   %echoaround% %victim% Several arrows sprout from %victim.name%'s chest.
   %send% %victim% You take several arrows in the chest.
else
  halt
~
#20110
super aggr arena trig~
0 g 100
~
if %actor.is_npc% != 0 || %actor.level% > 50
halt
else
murder %actor.name%
end
~
#20196
rogue spellup~
2 cd 100
gimme~
dg_cast 'sanc' %actor%
wait 5
dg_cast 'cleanse' %actor%
wait 5
dg_cast 'haste' %actor%
wait 5
dg_cast 'bless' %actor%
wait 5
dg_cast 'shield' %actor%
wait 5
dg_cast 'armor' %actor%
wait 5
dg_cast 'flame blade' %actor%
wait 5
dg_cast 'shadow vision' %actor%
wait 5
dg_cast 'true sight' %actor%
wait 5
dg_cast 'pulse heal' %actor%
wait 5
dg_cast 'pulse gain' %actor%
wait 5
dg_cast 'resist poison' %actor%
wait 5
dg_cast 'sagac' %actor%
wait 5
dg_cast 'fleet foot' %actor%
wait 5
dg_cast 'fly' %actor%
wait 5
dg_cast 'regen' %actor%
wait 5
dg_cast 'strength' %actor%
wait 5
dg_cast 'strength' %actor%
wait 5
dg_cast 'strength' %actor%
wait 5
dg_cast 'strength' %actor%
~
#20197
Black Respawn Protection~
2 g 100
~
if %actor.team% == black
 return 1
else
 %send% %actor% You are not on Black team.
 return 0
 end
~
#20198
Gold Respawn Protection~
2 g 100
~
if %actor.team% == gold
 return 1
else
 %send% %actor% You are not on Gold team.
 return 0
 end
~
$~
