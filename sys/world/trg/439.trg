#43901
khelban descript~
0 dg 1
samsera new thalos under dark lost city elven freehold freeport turra abenos shaden midgaard ~
if %speech%==samsera
wait 1
say I'll take you there right away, my old friend.
wait 1
%teleport% %actor% 18001
%force% %actor% look
end
if %speech%==turra
wait 1
say I'll take you there right away, my old friend.
wait 1
%teleport% %actor% 27718
%force% %actor% look
end
if %speech%==abenos
wait 1
say I'll take you there right away, my old friend.
wait 1
%teleport% %actor% 33138
%force% %actor% look
end
if %speech%==freeport
wait 1
say I'll take you there right away, my old friend.
wait 1
%teleport% %actor% 22953
%force% %actor% look
end
if %speech%==elven
wait 1
say I'll take you there right away, my old friend.
wait 1
%teleport% %actor% 30601
%force% %actor% look
end
if %speech%==under dark
wait 1
say I'll take you there right away, my old friend.
wait 1
%teleport% %actor% 20684
%force% %actor% look
end
if %speech%==Lost city
wait 1
say I'll take you there right away, my old friend.
wait 1
%teleport% %actor% 21201
%force% %actor% look
end
if %speech%==new thalos
wait 1
say I'll take you there right away, my old friend.
wait 1
%teleport% %actor% 13506
%force% %actor% look
end
if %speech%==freehold
wait 1
say I'll take you there right away, my old friend.
wait 1
%teleport% %actor% 12223
%force% %actor% look
end
if %speech%==shaden
wait 1
say I'll take you there right away, my old friend.
wait 1
%teleport% %actor% 11501
%force% %actor% look
end
~
#43902
mob trig~
0 g 100
~
say This place is not for the weak hearted, %actor.name% are you sure you want to go on?
~
#43903
new trigger~
0 e 0
says, 'yes'~
say Here take this.
give key %actor.name%
~
#43904
cuttrig~
1 ghi 100
drop get give~
test
~
#43905
area annoyance~
2 ab 100
~
say test
~
#43916
writhing mass guard trigger~
0 c 100
south~
%send% %actor% Suddenly the writhing mass of Darkness comes alive!
%echoaround% %actor% %actor.name% shudders and moves back as the Darkness blocks the path.
~
#43924
open door trig~
2 c 100
push~
%echoaround% %actor% %actor.name% leans against the Torch.
%send% %actor% All three doors in the room begin to creek.
wait 1 s
%send% %actor% Eeerie sounds can be heard coming from within each room.
wait 1 s
%send% %actor% Two decaying hands grab you from the darkness, Dragging you In.
%echoaround% %actor% %actor.name% Dissapears into the Darkness.
%teleport% %actor% 43926
%force% %actor% look
~
#43930
mobile corpse summon trigger~
0 h 100
~
if (%actor.is_npc% != 0 || %actor.level% > 50 || %actor.hitp% < 50) 
halt
else
say %actor.name%, Feel my wrath!
%echo% Torak, the blood God, raises his hands to the sky and utters the words, 'dgfhanbd'.
%echo% a disgustong zombie crawls its way from deep within the earth.
mload mob 43955
%force% assistant kill %actor.name%
end
~
#43948
blood trigger~
2 b 100
~
if %self.people%
eval victim %random.char%
if %victim.level% > 50
halt
end
if %victim.is_npc% != 0
halt
end  
%damage% %victim% %random.100%
%echoaround% %victim% blood starts to seep through %victim.name%'s clothes.
%send% %victim.name% You start to feel tired from the lack of blood.
else
halt
~
#43957
room giveway trig~
2 c 100
look~
%send% %actor% Suddenly the old wooden floorboards give way!
%echoaround% %actor% %actor.name% falls through the floor to a hidden cave below!
%teleport% %actor% 43958
wait 1
%force% %actor% look
%send% %actor% You notice small gashes from debree on your skin.
~
#43986
mobile load~
0 n 100
~
wait 10
%echo% A tortured prisoner shimmers into existence.
wait 5
say Where am i? What's going on here.
confuse
wait 3
Say Oh i was captured and tortured by Delgnar, I remember now.
wait 10
Say Thank you kind sir i am in your debt
end
~
#43994
shadowstep~
2 c 100
shadow shadowstep step shadow step south~
   %send% %actor% &07Suddenly the writhing mass of Darkness comes alive!&00
%echoaround% %actor% %actor.name% &07shudders and moves back as the Darkness blocks the path.&00
~
#43995
sham object trig~
1 b 100
~
if %self.name%
halt
if %victim.level% > 50
halt
end
if %victim.is_npc% != 0
halt
end  
%damage% %victim% %random.100%
%echoaround% %victim% Sand blasts towards%victim.name%.
%send% %victim.name% You begin to get stung by stand.
else
halt
~
#43996
disarm trigger~
1 b 100
disarmed~
if %self.name% /= %arg% && %arg.strlen% > 0
%echo% %self.shortdesc% Doesn't like being apart from his owner!
%load% object %self.vnum%
%purge% %self%
else
return 0
halt 
end
else
return 0
end
~
$~
