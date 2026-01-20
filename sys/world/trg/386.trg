#38600
Mage Portal~
2 d 100
Friend~
if %actor.class% == Magic User && %actor.align% < -250
  %echo% A disembodied voice says, "%actor.name% , your identity is accepted, your Evilness"
  %echoaround% %actor% %actor.name% seems to shrink to an infinitely small size, before disappearing.
  %send% %actor% You are sucked rapidly into oblivion, and deposited inside the guild.
  %teleport% %actor% 38602
  %force% %actor% look
  %echoaround% %actor% %actor.name% appears in a flash of light, and a cloud of choking black smoke.
elseif %actor.class% == Magic User && %actor.align% > 250
  %echo% A disembodied voice says, "%actor.name% , your identity is accepted, your Holiness"
  %echoaround% %actor% %actor.name% seems to shrink to an infinitely small size, before disappearing.
  %send% %actor% You are sucked rapidly into oblivion, and deposited inside the guild.
  %teleport% %actor% 38602
  %force% %actor% look
  %echoaround% %actor% %actor.name% appears in a flash of light, and a cloud of choking black smoke.
elseif %actor.class% == Magic User
  %echo% A disembodied voice says, "%actor.name% , your identity is accepted."
  %echoaround% %actor% %actor.name% seems to shrink to an infinitely small size, before disappearing.
  %send% %actor% You are sucked rapidly into oblivion, and deposited inside the guild.
  %teleport% %actor% 38602
%force% %actor% look
%echoaround% %actor% %actor.name% appears in a flash of light, and a cloud of choking black smoke.
else
  %echo% A disembodied voice says, "Begone, lest I transport you to the lowest depths of Hell, cretin!"
end
~
$~
