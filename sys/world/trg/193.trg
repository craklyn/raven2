#19302
Clan enter game~
0 e 0
has entered the game.~
wait 1 s
say &08Welcome back loyal clanmember &14%actor.name%&00, &08You have been missed!&00
end
~
#19303
honor clan set~
0 n 100
~
%force% honor set honor clan 2
~
#19304
courage teleport trigger~
0 d 100
help samsera new thalos under dark lost city elven freehold freeport turra abenos shaden midgaard~
if %speech%==samsera
  wait 1
  say &08F&00orever &08F&00orsaken, %actor.name%, Enjoy your trip!
  %force% %actor% dismount
  %teleport% %actor% 18001
  %force% %actor% look
end
if %speech%==turra
  wait 1
  say &08F&00orever &08F&00orsaken, %actor.name%, Enjoy your trip!
  wait 1
  %force% %actor% dismount
  %teleport% %actor% 27718
  %force% %actor% look
end
if %speech%==abenos
  wait 1
  say &08F&00orever &08F&00orsaken, %actor.name%, Enjoy your trip!
  wait 1
  %force% %actor% dismount
  %teleport% %actor% 33138
  %force% %actor% look
end
if %speech%==freeport
  wait 1
  say &08F&00orever &08F&00orsaken, %actor.name%, Enjoy your trip!
  wait 1
  %force% %actor% dismount
  %teleport% %actor% 22953
  %force% %actor% look
end
if %speech%==elven
wait 1
say &08F&00orever &08F&00orsaken, %actor.name%, Enjoy your trip!
wait 1
%force% %actor% dismount
%teleport% %actor% 30601
%force% %actor% look
end
if %speech%==under dark
  wait 1
  say &08F&00orever &08F&00orsaken, %actor.name%, Enjoy your trip!
  wait 1
  %force% %actor% dismount
  %teleport% %actor% 20684
  %force% %actor% look
end
if %speech%==Lost city
  wait 1
  say &08F&00orever &08F&00orsaken, %actor.name%, Enjoy your trip!
  wait 1
  %force% %actor% dismount
  %teleport% %actor% 21201
  %force% %actor% look
end
if %speech%==new thalos
  wait 1
  say &08F&00orever &08F&00orsaken, %actor.name%, Enjoy your trip!
  wait 1
  %force% %actor% dismount
  %teleport% %actor% 13506
  %force% %actor% look
end
if %speech%==freehold
  wait 1
  say &08F&00orever &08F&00orsaken, %actor.name%, Enjoy your trip!
  wait 1
  %force% %actor% dismount
  %teleport% %actor% 12223
  %force% %actor% look
end
if %speech%==shaden
  wait 1
  say &08F&00orever &08F&00orsaken, %actor.name%, Enjoy your trip!
  wait 1
  %force% %actor% dismount
  %teleport% %actor% 11501
  %force% %actor% look
end
if %speech%==midgaard
  wait 1
  say &08F&00orever &08F&00orsaken, %actor.name%, Enjoy your trip!
  wait 1
  %force% %actor% dismount
  %teleport% %actor% 3005
  %force% %actor% look
end
if %speech%==help
  wait 1
  mecho Courage says, '&15&07Say your city to be transported there.  Your choices are: Samsara, Midgaard, Turra, Abenos, Elven, Under Dark, Lost City, Freehold, New Thalos, Freeport, and Shaden.&00'
end
~
#19305
gangrel's demand~
1 c 100
dr~
if %cmd.mudcommand% == drink
%send% %actor% You forge your soul with your brothers of &08Forsaken&00.
end
return 0
~
#19306
clan goodbye test trigger~
0 c 100
rent~
%force% honor set honor clan 2
clan Forever Forsaken clanmember %actor.name%.
~
#19307
Teleport Trigger~
0 d 1
samsera new thalos under dark lost city elven freehold freeport turra abenos shaden midgaard help~
if %speech%==samsera
wait 1
say I'll take you there right away, my old friend.
wait 1
%force% %actor% dismount
%teleport% %actor% 18001
%force% %actor% look
end
if %speech%==turra
wait 1
say I'll take you there right away, my old friend.
wait 1
%force% %actor% dismount
%teleport% %actor% 27718
%force% %actor% look
end
if %speech%==abenos
wait 1
say I'll take you there right away, my old friend.
wait 1
%force% %actor% dismount
%teleport% %actor% 33138
%force% %actor% look
end
if %speech%==freeport
wait 1
say I'll take you there right away, my old friend.
wait 1
%force% %actor% dismount
%teleport% %actor% 22953
%force% %actor% look
end
if %speech%==elven
wait 1
say I'll take you there right away, my old friend.
wait 1
%force% %actor% dismount
%teleport% %actor% 30601
%force% %actor% look
end
if %speech%==under dark
wait 1
say I'll take you there right away, my old friend.
wait 1
%force% %actor% dismount
%teleport% %actor% 20684
%force% %actor% look
end
if %speech%==Lost city
wait 1
say I'll take you there right away, my old friend.
wait 1
%force% %actor% dismount
%teleport% %actor% 21201
%force% %actor% look
end
if %speech%==new thalos
wait 1
say I'll take you there right away, my old friend.
wait 1
%force% %actor% dismount
%teleport% %actor% 13506
%force% %actor% look
end
if %speech%==freehold
wait 1
say I'll take you there right away, my old friend.
wait 1
%force% %actor% dismount
%teleport% %actor% 12223
%force% %actor% look
end
if %speech%==shaden
wait 1
say I'll take you there right away, my old friend.
wait 1
%force% %actor% dismount
%teleport% %actor% 11501
%force% %actor% look
end
if %speech%==midgaard
wait 1
say I'll take you there right away, my old friend.
wait 1
%force% %actor% dismount
%teleport% %actor% 3005
%force% %actor% look
end
if %speech%==help
wait 1
say Say your city to be transported there.  Your choices are: Samsara, Midgaard, Turra, Abenos, Elven, Under Dark, Lost City, Freehold, New Thalos, Freeport, and Shaden.
end
~
#19308
OD teleport to clan trigger~
2 c 100
od~
   if %actor.clan% != 11
     halt
   end 
   %send% %actor% The &07darkness&00 comes alive around you.
   %send% %actor% You begin to dematerialize as dark forces devastate your flesh!
   %teleport% %actor% 19385
   %force% %actor% look
~
#19309
Outlaw Teleport Trigger~
0 d 1
samsera new thalos under dark lost city elven freehold freeport turra abenos shaden midgaard help~
if %speech%==samsera
wait 1
say I'll take you there right away, my old friend.
wait 1
%force% %actor% dismount
%teleport% %actor% 18001
%force% %actor% look
end
if %speech%==turra
wait 1
say I'll take you there right away, my old friend.
wait 1
%force% %actor% dismount
%teleport% %actor% 27718
%force% %actor% look
end
if %speech%==abenos
wait 1
say I'll take you there right away, my old friend.
wait 1
%force% %actor% dismount
%teleport% %actor% 33138
%force% %actor% look
end
if %speech%==freeport
wait 1
say I'll take you there right away, my old friend.
wait 1
%force% %actor% dismount
%teleport% %actor% 22953
%force% %actor% look
end
if %speech%==elven
wait 1
say I'll take you there right away, my old friend.
wait 1
%force% %actor% dismount
%teleport% %actor% 30601
%force% %actor% look
end
if %speech%==under dark
wait 1
say I'll take you there right away, my old friend.
wait 1
%force% %actor% dismount
%teleport% %actor% 20684
%force% %actor% look
end
if %speech%==Lost city
wait 1
say I'll take you there right away, my old friend.
wait 1
%force% %actor% dismount
%teleport% %actor% 21201
%force% %actor% look
end
if %speech%==new thalos
wait 1
say I'll take you there right away, my old friend.
wait 1
%force% %actor% dismount
%teleport% %actor% 13506
%force% %actor% look
end
if %speech%==freehold
wait 1
say I'll take you there right away, my old friend.
wait 1
%force% %actor% dismount
%teleport% %actor% 12223
%force% %actor% look
end
if %speech%==shaden
wait 1
say I'll take you there right away, my old friend.
wait 1
%force% %actor% dismount
%teleport% %actor% 11501
%force% %actor% look
end
if %speech%==midgaard
wait 1
say I'll take you there right away, my old friend.
wait 1
%force% %actor% dismount
%teleport% %actor% 3005
%force% %actor% look
end
if %speech%==help
wait 1
say Say your city to be transported there.  Your choices are: Samsara, Midgaard, Turra, Abenos, Elven, Under Dark, Lost City, Freehold, New Thalos, Freeport, and Shaden.
end
~
#19310
Wanderers teleport trigger~
0 d 100
samsera new thalos under dark lost city elven freehold freeport turra abenos shaden midgaard help~
   if %speech%==samsera
wait 1
say I'll take you there right away, my old friend.
wait 1
%force% %actor% dismount
%teleport% %actor% 18001
%force% %actor% look
end
   if %speech%==turra
wait 1
say I'll take you there right away, my old friend.
wait 1
%force% %actor% dismount
%teleport% %actor% 27718
%force% %actor% look
end
   if %speech%==abenos
wait 1
say I'll take you there right away, my old friend.
wait 1
%force% %actor% dismount
%teleport% %actor% 33138
%force% %actor% look
end
   if %speech%==freeport
wait 1
say I'll take you there right away, my old friend.
wait 1
%force% %actor% dismount
%teleport% %actor% 22953
%force% %actor% look
end
   if %speech%==elven
wait 1
say I'll take you there right away, my old friend.
wait 1
%force% %actor% dismount
%teleport% %actor% 30601
%force% %actor% look
end
   if %speech%==under dark
wait 1
say I'll take you there right away, my old friend.
wait 1
%force% %actor% dismount
%teleport% %actor% 20684
%force% %actor% look
end
   if %speech%==Lost city
wait 1
say I'll take you there right away, my old friend.
wait 1
%force% %actor% dismount
%teleport% %actor% 21201
%force% %actor% look
end
   if %speech%==new thalos
wait 1
say I'll take you there right away, my old friend.
wait 1
%force% %actor% dismount
%teleport% %actor% 13506
%force% %actor% look
end
   if %speech%==freehold
wait 1
say I'll take you there right away, my old friend.
wait 1
%force% %actor% dismount
%teleport% %actor% 12223
%force% %actor% look
end
   if %speech%==shaden
wait 1
say I'll take you there right away, my old friend.
wait 1
%force% %actor% dismount
%teleport% %actor% 11501
%force% %actor% look
end
   if %speech%==midgaard
wait 1
say I'll take you there right away, my old friend.
wait 1
%force% %actor% dismount
%teleport% %actor% 3005
%force% %actor% look
end
   if %speech%==help
wait 1
say Say your city to be transported there.  Your choices are: Samsara, Midgaard, Turra, Abenos, Elven, Under Dark, Lost City, Freehold, New Thalos, Freeport, and Shaden.
end
~
#19311
Coven teleport trigger~
0 d 100
samsera new thalos under dark lost city elven freehold freeport turra abenos shaden midgaard help~
   if %speech%==samsera
wait 1
say I'll take you there right away, my old friend.
wait 1
%force% %actor% dismount
%teleport% %actor% 18001
%force% %actor% look
end
   if %speech%==turra
wait 1
say I'll take you there right away, my old friend.
wait 1
%force% %actor% dismount
%teleport% %actor% 27718
%force% %actor% look
end
   if %speech%==abenos
wait 1
say I'll take you there right away, my old friend.
wait 1
%force% %actor% dismount
%teleport% %actor% 33138
%force% %actor% look
end
   if %speech%==freeport
wait 1
say I'll take you there right away, my old friend.
wait 1
%force% %actor% dismount
%teleport% %actor% 22953
%force% %actor% look
end
   if %speech%==elven
wait 1
say I'll take you there right away, my old friend.
wait 1
%force% %actor% dismount
%teleport% %actor% 30601
%force% %actor% look
end
   if %speech%==under dark
wait 1
say I'll take you there right away, my old friend.
wait 1
%force% %actor% dismount
%teleport% %actor% 20684
%force% %actor% look
end
   if %speech%==Lost city
wait 1
say I'll take you there right away, my old friend.
wait 1
%force% %actor% dismount
%teleport% %actor% 21201
%force% %actor% look
end
   if %speech%==new thalos
wait 1
say I'll take you there right away, my old friend.
wait 1
%force% %actor% dismount
%teleport% %actor% 13506
%force% %actor% look
end
   if %speech%==freehold
wait 1
say I'll take you there right away, my old friend.
wait 1
%force% %actor% dismount
%teleport% %actor% 12223
%force% %actor% look
end
   if %speech%==shaden
wait 1
say I'll take you there right away, my old friend.
wait 1
%force% %actor% dismount
%teleport% %actor% 11501
%force% %actor% look
end
   if %speech%==midgaard
wait 1
say I'll take you there right away, my old friend.
wait 1
%force% %actor% dismount
%teleport% %actor% 3005
%force% %actor% look
end
   if %speech%==help
wait 1
say Say your city to be transported there.  Your choices are: Samsera, Midgaard, Turra, Abenos, Elven, Under Dark, Lost City, Freehold, New Thalos, Freeport, and Shaden.
end
~
#19313
sparring partner calm~
0 d 100
recall ~
dg_cast 'word of reca' %actor.name%
~
#19314
Coven enter clan trigger~
2 d 100
horse~
if %actor.clan% != 6
      %send% %actor% What!  You must be drunk!  Back to the dump where you belong, Hobo!
      %damage% %actor% 100
      %force% %actor% dismount
      %teleport% %actor% 3030
      halt
      end 
      %send% %actor% A raging vortex composed of dark energy consumes your body, burning and thrashing your flesh until you dematerialize.
      %force% %actor% dismount
      %teleport% %actor% 19344
      %force% %actor% look
      %echo% %actor.name% phases out of existence.
~
#19315
Teleport Into Clan (Brethren)~
2 d 100
bruh~
if %actor.clan% != 8
%send% %actor% What!  You must be drunk!  Back to the dump where you belong, Hobo!
%damage% %actor% 100
%force% %actor% dismount
%teleport% %actor% 3030
halt
end 
 
%send% %actor% A raging vortex composed of dark energy consumes your body, burning and thrashing your flesh until you dematerialize.
%force% %actor% dismount
%teleport% %actor% 19373
%force% %actor% look
%echo% %actor.name% phases out of existence.
~
#19391
pit fiends trigger~
0 dg 1
samsera new thalos under dark lost city elven freehold freeport turra abenos shaden midgaard~
if %speech%==samsera
wait 1
say Okay, %actor.name%, you're lucky I don't send you to Hell.
wait 1
%force% %actor% dismount
%teleport% %actor% 18001
%force% %actor% look
end
if %speech%==turra
wait 1
say Okay, %actor.name%, you're lucky I don't send you to Hell.
wait 1
%force% %actor% dismount
%teleport% %actor% 27718
%force% %actor% look
end
if %speech%==abenos
wait 1
say Okay, %actor.name%, you're lucky I don't send you to Hell.
wait 1
%force% %actor% dismount
%teleport% %actor% 33138
%force% %actor% look
end
if %speech%==freeport
wait 1
say Okay, %actor.name%, you're lucky I don't send you to Hell.
wait 1
%force% %actor% dismount
%teleport% %actor% 22953
%force% %actor% look
end
if %speech%==elven
wait 1
say Okay, %actor.name%, you're lucky I don't send you to Hell.
wait 1
%force% %actor% dismount
%teleport% %actor% 30601
%force% %actor% look
end
if %speech%==under dark
wait 1
say Okay, %actor.name%, you're lucky I don't send you to Hell.
wait 1
%force% %actor% dismount
%teleport% %actor% 20684
%force% %actor% look
end
if %speech%==Lost city
wait 1
say Okay, %actor.name%, you're lucky I don't send you to Hell.
wait 1
%force% %actor% dismount
%teleport% %actor% 21201
%force% %actor% look
end
if %speech%==new thalos
wait 1
say Okay, %actor.name%, you're lucky I don't send you to Hell.
wait 1
%force% %actor% dismount
%teleport% %actor% 13506
%force% %actor% look
end
if %speech%==freehold
wait 1
say Okay, %actor.name%, you're lucky I don't send you to Hell.
wait 1
%force% %actor% dismount
%teleport% %actor% 12223
%force% %actor% look
end
if %speech%==shaden
wait 1
say Okay, %actor.name%, you're lucky I don't send you to Hell.
wait 1
%force% %actor% dismount
%teleport% %actor% 11501
%force% %actor% look
end
if %speech%==midgaard
wait 1
say Okay, %actor.name%, you're lucky I don't send you to Hell.
wait 1
%force% %actor% dismount
%teleport% %actor% 3005
%force% %actor% look
end
~
#19392
new trigger~
0 dg 100
samsera~
*hopefully a sk/dk dismount trigger for teleport
eval class %self.class%
eval level %self.level%
eval align %self.align%
eval victim %actor%
if %victim.is_npc% != 0 || %victim.level% > 50
%send% %self% %victim.name% is an immortal, or NPC
halt
else
%send% %self% %victim.name% is a valid target
switch %class%
if %speech%==samsera
wait 1
say Okay, %actor.name%, you're lucky I don't send you to Hell.
wait 1
case Solamnic Knight
* Do what I do best
if %level% < 9
%force% %actor% dismount
elseif %level% >= 9 && %level% < 44 && %self.eq(19)%
%force% %actor% dismount
elseif %level% > 44 && %actor.align% < -250
%force% %actor% dismount
end
~
$~
