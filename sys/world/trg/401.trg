#40167
Scarab of Insanity~
1 j 100
~
if %actor.is_npc% != 0
  halt
else
  eval wearer %actor%
  global wearer
  otimer %random.3%
end
if %actor.is_npc% = 0
  eval owner %actor%
  global owner
else
  halt
 end
~
#40168
Insanity targeting~
1 f 100
~
if %wearer%
  %echoaround% %wearer% %wearer.name% appears to have completely lost it!
  %send% %wearer% You feel a bit strange.
  %force% %wearer% murder %random.3%.
  otimer %random.3%
else if %owner%
  %send% %owner% %self.name% rattles around in your pack, restlessly.
else
   halt
end
~
#40169
Insanity trigger2~
1 f 100
~
if %wearer%
   %echoaround% %wearer% %wearer.name% appears to have completely lost it!
   %send% %wearer% You feel a bit strange.
   %force% %wearer% murder %random.3%.
   otimer %random.3%
else if %owner%
  %send% %owner% %self.name% rattles around in your pack, restlessly.
else
   halt
end
~
#40170
Insanity Owner tracking~
1 g 100
~
if %actor.is_npc% = 0
  eval owner %actor%
  global owner
else
halt
end
~
#40177
Pull chain teleporter~
2 c 100
pull~
* command script for room 40177
%echoaround% %actor% You here a click as %actor.name% pulls on the chain.
%send% %actor% You hear a click as the chain moves.
%echoaround% %actor% %actor.name% fades from your view.
%send% %actor% Your body is jerked so hard that your teeth rattle!
%teleport% %actor% 41000
%echoaround% %actor% %actor.name% drops from the sky.
%send% %actor% You land face down in the dirt.
%force% %actor% look
~
$~
