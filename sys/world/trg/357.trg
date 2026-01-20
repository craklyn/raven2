#35701
Queen's Greeting~
0 g 100
~
   if %actor.is_npc% != 0
halt
end 
dg_cast 'dance of dreams' %actor.name%
~
#35702
Queen Aggro~
0 g 100
~
   if %actor.is_npc% != 0
halt
end 
kill %actor.name%
~
#35703
Room_sleeper~
2 g 100
~
   if %actor.is_npc% != 0
halt
end 
dg_cast 'dance of dreams' %actor.name%
~
#35704
guardmob~
0 c 100
e~
if %actor.class% == mage
 %force% %actor.name% east
elseif %actor.race% == amara
 force em east
else
 say Get lost!
end
~
#35740
guard trigger~
0 i 100
~
if %target.is_npc%
  halt
elseif %actor.race% != Amara && %actor.race% != Elemental && %actor.race != Magic User
  return 0
end 
~
$~
