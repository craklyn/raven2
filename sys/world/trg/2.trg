#221
mising trigger 1~
2 g 100
~
say This is a missing trigger, please report this room to an imm thank you
~
#222
missing trigger 2~
2 g 100
~
say this is a missing trigger please contact imm thank you
~
#224
missing trigger~
2 g 100
~
say this is a missing trigger please contact an imm thank you
~
#226
missing trigger~
2 g 100
~
say this is a missing trigger please contact an imm thank you
~
#227
missing trigger~
2 g 100
~
say this is a missing trigger please contact an imm thank you
~
#309
missing trigger~
2 g 100
~
say this is a missing trigger please contact an imm thank you
~
#700
Test object_speech trigger~
1 c 100
city~
%echo% %actor.name% vanishes in a &25&07puff&00 of smoke.
wait 1
%force% %actor% dismount
%teleport% %actor% 18001
%force% %actor% look
end
~
#801
mob death 1 qp reward~
0 f 100
~
; establish the group master
; this will escalate past ungrouped charmies
eval master %actor% 
eval more 1
while %more%
  if %master.master% && (%master.is_npc% || %master.is_grouped%)
    eval master %master.master%
  else
    eval more 0
  end
done
; reward the master
if %master.is_npc% == 0 && %master.room% == %actor.room% && %master.is_grouped%
  %send% %master% &13You have earned 1 qp!&00
  nop %master.qp(1)%
end
; step through the followers to reward them
eval follower 0
meta ch master.follower(%follower%)
while %ch%
  if %ch.is_npc% == 0 && %ch.room% == %actor.room% && %ch.is_grouped%
    %send% %ch% &13You have earned 1 qp!&00
    nop %ch.qp(1)%
  end
  eval follower %follower% + 1
  meta ch master.follower(%follower%)
done
~
#802
mob death 2 qp reward~
0 f 100
~
; establish the group master
; this will escalate past ungrouped charmies
eval master %actor% 
eval more 1
while %more%
  if %master.master% && (%master.is_npc% || %master.is_grouped%)
    eval master %master.master%
  else
    eval more 0
  end
done
; reward the master
if %master.is_npc% == 0 && %master.room% == %actor.room% && %master.is_grouped%
  %send% %master% &13You have earned 2 qp!&00
  nop %master.qp(2)%
end
; step through the followers to reward them
eval follower 0
meta ch master.follower(%follower%)
while %ch%
  if %ch.is_npc% == 0 && %ch.room% == %actor.room% && %ch.is_grouped%
    %send% %ch% &13You have earned 2 qp!&00
    nop %ch.qp(2)%
  end
  eval follower %follower% + 1
  meta ch master.follower(%follower%)
done
~
#803
mob death 3 qp reward~
0 f 100
~
; establish the group master
; this will escalate past ungrouped charmies
eval master %actor% 
eval more 1
while %more%
  if %master.master% && (%master.is_npc% || %master.is_grouped%)
    eval master %master.master%
  else
    eval more 0
  end
done
; reward the master
if %master.is_npc% == 0 && %master.room% == %actor.room% && %master.is_grouped%
  %send% %master% &13You have earned 3 qp!&00
  nop %master.qp(3)%
end
; step through the followers to reward them
eval follower 0
meta ch master.follower(%follower%)
while %ch%
  if %ch.is_npc% == 0 && %ch.room% == %actor.room% && %ch.is_grouped%
    %send% %ch% &13You have earned 3 qp!&00
    nop %ch.qp(3)%
  end
  eval follower %follower% + 1
  meta ch master.follower(%follower%)
done
~
#804
mob death 4 qp reward~
0 f 100
~
; establish the group master
; this will escalate past ungrouped charmies
eval master %actor% 
eval more 1
while %more%
  if %master.master% && (%master.is_npc% || %master.is_grouped%)
    eval master %master.master%
  else
    eval more 0
  end
done
; reward the master
if %master.is_npc% == 0 && %master.room% == %actor.room% && %master.is_grouped%
  %send% %master% &13You have earned 4 qp!&00
  nop %master.qp(4)%
end
; step through the followers to reward them
eval follower 0
meta ch master.follower(%follower%)
while %ch%
  if %ch.is_npc% == 0 && %ch.room% == %actor.room% && %ch.is_grouped%
    %send% %ch% &13You have earned 4 qp!&00
    nop %ch.qp(4)%
  end
  eval follower %follower% + 1
  meta ch master.follower(%follower%)
done
~
#805
mob death 5 qp reward~
0 f 100
~
; establish the group master
; this will escalate past ungrouped charmies
eval master %actor% 
eval more 1
while %more%
  if %master.master% && (%master.is_npc% || %master.is_grouped%)
    eval master %master.master%
  else
    eval more 0
  end
done
; reward the master
if %master.is_npc% == 0 && %master.room% == %actor.room% && %master.is_grouped%
  %send% %master% &13You have earned 5 qp!&00
  nop %master.qp(5)%
end
; step through the followers to reward them
eval follower 0
meta ch master.follower(%follower%)
while %ch%
  if %ch.is_npc% == 0 && %ch.room% == %actor.room% && %ch.is_grouped%
    %send% %ch% &13You have earned 5 qp!&00
    nop %ch.qp(5)%
  end
  eval follower %follower% + 1
  meta ch master.follower(%follower%)
done
~
#806
mob death 1 qp reward NOT SHARED TO GROUP~
0 f 100
~
   
establish the group master
; this will escalate past ungrouped charmies
eval master %actor% 
eval more 1
while %more%
if %master.master% && (%master.is_npc% || %master.is_grouped%)
eval master %master.master%
else
eval more 0
end
done
; reward the master
if %master.is_npc% == 0 && %master.room% == %actor.room% && %master.is_grouped%
%send% %master% You have earned 1 qp!
nop %master.qp(1)%
end
~
$~
