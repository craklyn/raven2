#35400
hasana's firecracker~
1 h 100
~
%echo% %actor.name% lights the fuse on a firecracker!
wait 1 second
%echo%  &30  &13.&01.    &02.     &04. &05.   &06. &08.
%echo%      &30 &02.    &04. &05.  &06+    &08.  &09. &10.
%echo% &03+&11'  &10.  &09+  &08" &06.   &05. &04!
%echo% &30 &04+ &05. &06'   &08+ &09! &10. &11+ &12, &13+
%echo% &30&08.  &09. &10. &06+  &08+     &09+    &10'   &11.  &12+
%echo% &30&13!      &01'&02.  &04+  &05+  &06'   &08+ &08.  &09. &10.
%echo% &30&08.  &09. &10. &06+    &07'  &08+ &09+  &10+  &11.  &12"
%echo% &30&10+  &09'  &08+ &10-= &25 BOOM  &00&10 =-   &05+  &04.&02'
%echo% &30&11'  &12'  &13. &02. &04+   &05+ &08.  &09. &10.
%echo% &30&06+    &08.  &09+  &10+ &11+ &12'  &13+
%echo% &30 &04+  &05!  &06,   &08'  &09' &11+ &12+ &08.  &09. &10.
%echo%     &30  &09.  &10+    &11'   &12+   &13+  &01'
%echo% &30&06! &08,     &09+   &10+    &11+  &12. &13,
%echo% &30 &09.  &10+    &11'   &12+   &13+  &01'&00
%echo% &30 &09.  &10+    &11'   &12+   &13+  &01'&00
dg_cast 'earthquake'
otransform 36498
~
#35403
Octypus dream~
0 g 100
~
if %target.is_npc%
halt
elseif %actor.race% == Amara || %actor.race% == Elemental || %actor.class% == Magic User
give pearl %actor.name%
 %echo% Your key master.
else
dg_cast 'dance of dreams' %actor%
end
~
#35404
coral damage~
2 b 50
~
eval number %room.people%
if %number% > 6
eval person %random.char%
%damage% %person% %random.100%
%send% %person% in the crowded confines you get jostled into the sharp coral.
end
~
#35411
randomizer 35411~
2 f 100
~
switch %random.3%
case 1
wdoor %self.vnum% east room 35416
wdoor %self.vnum% south room 35412
wdoor %self.vnum% west room 35407
break
case 2
wdoor %self.vnum% east room 35407
wdoor %self.vnum% south room 35416
wdoor %self.vnum% west room 35412
break
case 3
wdoor %self.vnum% east room 35412
wdoor %self.vnum% south room 35407
wdoor %self.vnum% west room 35416
break
~
#35412
randomizer 35412~
2 f 100
~
switch %random.4%
case 1
wdoor %self.vnum% north room 35411
wdoor %self.vnum% east room 35417
wdoor %self.vnum% south room 35413
wdoor %self.vnum% west room 35408
break
case 2
wdoor %self.vnum% east room 35411
wdoor %self.vnum% south room 35417
wdoor %self.vnum% west room 35413
wdoor %self.vnum% north room 35408
break
case 3
wdoor %self.vnum% south room 35411
wdoor %self.vnum% west room 35417
wdoor %self.vnum% north room 35413
wdoor %self.vnum% east room 35408
break
case 4
wdoor %self.vnum% west room 35411
wdoor %self.vnum% north room 35417
wdoor %self.vnum% east room 35413
wdoor %self.vnum% south room 35408
break
~
#35416
random exits for elms area~
2 f 100
~
switch %random.4%
case 1
wdoor %self.vnum% north room 35415
wdoor %self.vnum% east room 35407
wdoor %self.vnum% south room 35417
wdoor %self.vnum% west room 35411
break
case 2
wdoor %self.vnum% east room 35415
wdoor %self.vnum% south room 35407
wdoor %self.vnum% west room 35417
wdoor %self.vnum% north room 35411
break
case 3
wdoor %self.vnum% south room 35415
wdoor %self.vnum% west room 35407
wdoor %self.vnum% north room 35417
break
case 4
wdoor %self.vnum% west room 35415
wdoor %self.vnum% north room 35407
wdoor %self.vnum% east room 35411
wdoor %self.vnum% south room 35417
break
~
#35420
cast poison~
0 g 100
~
if %actor.is_npc% != 0 || %actor.level% > 50
halt
else
dg_cast 'Poison' %actor.name%
end
~
#35421
curse trigger~
0 g 100
~
if %actor.is_npc% != 0 || %actor.level% > 50
halt
else
dg_cast 'curse' %actor.name%
end
~
#35432
coral tunnel damage~
2 g 100
~
if %actor.is_npc%
halt
wait 1
else %echo% As you tryto squeeze through the sharp coral tears at your tender flesh.
dg_cast 'poison' %actor%
%damage% %actor% %random.50%
end
~
#35433
eel shock~
0 k 75
~
dg_cast 'chain ligh'
~
#35451
amara janitor~
0 b 1
~
%teleport% %self% 35483
give all anemone
eval cash %self.gold%
give %cash% coins anemone
wait 1 s
u
switch %random.3%
case 1
shout Now stop littering my fair city!
break
case 2
shout Durn kids! pick up after yourselfs.
break
case 3
mumble
break
~
#35471
bribe offer~
0 g 100
~
if %actor.is_npc% != 0 || %actor.level% > 50
halt
else
tell %actor.name% The hinges are a bit rusty. They might need some "grease"..
end
~
#35472
amaraa bribe~
0 m 350000
~
say Your gift is accepted.
%load% obj 35473
unlock energy
wait 1 s
open energy
say Hustle through, I aint got all day.
wait 10 s
close energy
lock energy
wait 1 s 
s
wait 1 s
s
wait 1 s
s
wait 1 s
d
eval gold %self.gold%
give %gold% coins anemone
give anem 350000 coins
junk key
wait 1 s
u
wait 1 s
n
wait 1 s
n
wait 1 s
n
~
#35476
testing gold flag~
2 g 100
~
eval gold %actor.gold%
if %actor.is_npc% != 0
%force% %actor% drop %gold% coins
end
~
#35477
Protect the citizens~
0 k 75
~
%load% mob 35439
%echo% Guards Help! Thieves! Murderers!
wait 1
%force% guard rescue citizen
~
#35478
shark feed~
0 f 60
~
%echo% &19
%echo% So much blood in the water has drawn some unexpected visitors.
%echo% &00
switch %random.4%
wait 2 s
case 1
%load% mob 35400
break
case 2
%load% mob 35400
%load% mob 35400
break
case 3
%load% mob 35400
%load% mob 35400
%load% mob 35400
break
case 4
%load% mob 35400
%load% mob 35400
%load% mob 35400
%load% mob 35400
break
~
#35483
para trigger~
0 k 75
~
wait 1
eval target %random.char%
dg_cast 'paralyze' %target.name%
  dg_cast 'poison' %target.name%
dg_cast 'paralyze' %target.name%
dg_cast 'paralyze' %target.name%
~
#35491
amara dancers sleep~
0 k 50
~
wait 1
cast 'dance of dreams' %actor.name%
~
#35492
noconvert~
0 c 100
conver~
say How DARE you do that in my presence! So be it petty mortal.
eval align %self.align%
if %self.align% >0
mtransform -35490
else
mtransform -35489
end
kill %actor%
~
#35499
testing~
0 k 50
~
eval number %people.35483%
eval target %random.char%
if %target.is_npc% || %target.level% > 50 || %number% <9
halt
else
%echoaround% %target.name% Is Tossed from the room violently by the swirling currents caused by the fight.
%teleport% %target% 35482
%send% %target% The wild currents cause by the fight buffet you out of the cavern!
end
~
$~
