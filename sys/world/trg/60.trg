#6001
Thog Entrance~
2 c 100
e~
if %actor.name% != Thog
  %send% %actor% Nice try, but this isn't your house!
  %damage% %actor% 100
  %force% %actor% dismount
  %teleport% %actor% 3030
  %echo% %actor.name% suddenly falls into a hole!
  %force %actor% look
else
  return 0
end
~
$~
