#38900
Assassin Portal~
2 c 100
e~
if %cmd.mudcommand% != enter || %arg% != trap
  halt
end
%echoaround% %actor% %actor.name% slips into the shadows and vanishes before your eyes.
if %actor.class% == Assassin
  %send% %actor% The trap closes over your head, suffocating you in death and darkness.
  %send% %actor% You awaken to find yourself in a world of intrigue and death.
  %teleport% %actor% 38902
  %force% %actor% look
  %echoaround% %actor% %actor.name% slips quietly into the guild.
else
  %send% %actor% As the trap closes over your head, you realize you've made a grave mistake.  You beg to the gods for you life, and realize you have been spared.
  %force% %actor% look
end
~
#38901
Waken on Entry~
2 g 100
~
%force% all wake
%force% all stand
%echo% Someone is entering the room!
~
#38902
Assassin Exit script~
2 c 100
enter~
if %cmd.mudcommand% != enter || %arg% != trap
  halt
end
%echoaround% %actor% %actor.name% slips into the shadows and vanishes before your eyes.
if %actor.class% == Assassin
  %send% %actor% The trap opens over your head, lifting you into the light.
  %send% %actor% You awaken to find yourself in the outside world.
  %teleport% %actor% 38901
  %force% %actor% look
  %echoaround% %actor% %actor.name% slips quietly out of the guild.
else
  %send% %actor% As the trap closes over your head, you realize you've made a grave mistake.  You beg to the gods for you life, and realize you have been spared.
  %force% %actor% look
end
~
$~
