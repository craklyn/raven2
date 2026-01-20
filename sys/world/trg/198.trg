#19800
Close Rift --> Attached to room 19801~
2 g 100
none~
if %actor.level% > 50
halt
else
If %direction% == up
wait 1 s
%send% %actor% You arrive in a small passageway. The rift above slowly closes as the fog settles in.
%echoaround% %actor% As %actor.name% arrives, the fog above slowly closes up the rift.
wait 1 s
%door% 13583 down flags ab
%door% 19801 up flags ab
end
halt
end if
~
#19801
new trigger~
0 dg 1
cheers~
if %speech%==cheers
wait 1
say Have a good time, %actor.name%!
wait 1
   %force% %actor% dismount
   %teleport% %actor% 180
   %force% %actor% look
   end
~
#19810
trick or treater halloween trigger~
0 f 100
~
switch %random.20%
  case 1
    %load% obj 19899
    %echo% A &07trick&00 or &08treater&00 howls as their life force drains away!
  break
  case 2
    %load% obj 1412
    %echo% A &07trick&00 or &08treater&00 howls as their life force drains away!
  break
  case 3
    %load% obj 1691
    %echo% A &07trick&00 or &08treater&00 howls as their life force drains away!
  break
  case 4
    %load% obj 1692
    %echo% A &07trick&00 or &08treater&00 howls as their life force drains away!
  break
  case 5
    %load% obj 1693
    %echo% A &07trick&00 or &08treater&00 howls as their life force drains away!
  break
  case 6
    %load% obj 37168
    %echo% A &07trick&00 or &08treater&00 howls as their life force drains away!
  break
  case 7
    %load% obj 19899
    %echo% A &07trick&00 or &08treater&00 howls as their life force drains away!
  break
  case 8
    %load% obj 1412
    %echo% A &07trick&00 or &08treater&00 howls as their life force drains away!
  break
  case 9
    %load% obj 1691
    %echo% A &07trick&00 or &08treater&00 howls as their life force drains away!
  break
  case 10
    %load% obj 1692
    %echo% A &07trick&00 or &08treater&00 howls as their life force drains away!
  break
  case 11
    %load% obj 1693
    %echo% A &07trick&00 or &08treater&00 howls as their life force drains away!
  break
  case 12
    %load% obj 19899
    %echo% A &07trick&00 or &08treater&00 howls as their life force drains away!
  break
  case 13
    %load% obj 1412
    %echo% A &07trick&00 or &08treater&00 howls as their life force drains away!
  break
  case 14
    %load% obj 1691
    %echo% A &07trick&00 or &08treater&00 howls as their life force drains away!
  break
  case 15
    %load% obj 1692
    %echo% A &07trick&00 or &08treater&00 howls as their life force drains away!
  break
  case 16
    %load% obj 1693
    %echo% A &07trick&00 or &08treater&00 howls as their life force drains away!
  break
  case 17
    %load% obj 37168
    %echo% A &07trick&00 or &08treater&00 howls as their life force drains away!
  break
  case 18
    %load% obj 1412
    %echo% A &07trick&00 or &08treater&00 howls as their life force drains away!
  break
  case 19
    %load% obj 1691
    %echo% A &07trick&00 or &08treater&00 howls as their life force drains away!
  break
  case 20
    %load% obj 1693
    %echo% A &07trick&00 or &08treater&00 howls as their life force drains away!
  break
  default
    %echo% This is broken, you don't get SH*T!
  break
done
~
#19811
halloween candle loader~
0 f 100
~
eval candleload 77.4%
if %candleoad% ==1
  %load% obj 33182
  return 0
elseif %candleload% ==2
  %load% obj 33183
  return 0
elseif %candleload% ==3
  %load% obj 33183
  return 0
elseif %candleload% ==4
  %load% obj 33184
  return 0
end
~
#19814
Genesis officer trigger~
2 d 100
lameass~
if (%actor.is_npc% != 0 || %actor.level% > 50)
         halt
      else
         %echo% %actor.name% is surrounded by a dark mist and vanishes.
         wait 1
         %force% %actor% dismount
         %teleport% %actor% 19812
         %force% %actor% look
      end
~
#19815
Exit trigger -->  Takes to room 19803~
2 d 100
exit~
%echo% %actor.name% is surrounded by a swirling mass of energy and quickly disappears.
wait 1
%force% %actor% dismount
%teleport% %actor% 19803
%force% %actor% look
end
~
#19816
cheers_trigger~
2 d 100
cheers~
%echo% %actor.name% is surrounded by a &07swirling mass&00 of &07darkness&00, &25&08levatates&00 off the ground and vanishes.
wait 1
%force% %actor% dismount
%teleport% %actor% 180
%force% %actor% look
end
~
#19817
faith_trigger~
2 d 100
faith~
%echo% %actor.name% is surrounded by a &07swirling mass&00 of &07darkness&00, &25&08levatates&00 off the ground and vanishes.
wait 1
%force% %actor% dismount
%teleport% %actor% 19816
%force% %actor% look
end
~
#19818
cave_trigger~
2 g 100
none~
if %actor.level% > 50
halt
else
If %direction% == up
wait 1 s
%send% %actor% You arrive in a &13cold&07 cave&00 room filled with mist.  The cave door above you slowly closes as the mist settles in.
%echoaround% %actor% As %actor.name% arrives, the cave door slowly begins to close.
wait 1 s
%door% 1699 down flags ab
%door% 19813 up flags ab
end
halt
end if
~
#19819
duncas load trigger~
2 cd 100
reload~
if %speech%==reload
%echo% &08D&00unca&08s&00 has arrived and prepares for his next match.
%load% mob 19806
end
~
#19820
duncas rewield trigger~
2 cd 100
rewield~
if %speech%==rewield
%load% object 37802
wait 1
%force% duncas get short
wait 2
%force% duncas say Who left my sword lying in the dirt?!?
%force% duncas wield short
end
%force% duncas say Prepare to feel cold steel my friend!
end
~
#19821
wield trigger~
0 k 100
~
context %self.id%
if (%already_fighting%)
wait 10
wield short
end
~
#19822
Gutter's transport trigger~
2 d 100
faith~
if %actor.clan% != 15
%send% %actor% You are not &25&10holy&00 enough to be presented with such a fine gift.  Go back to &08hell&00 where you belong!
%damage% %actor% 150
%teleport% %actor% 3030
halt
end
%send% %actor% The &03Earth&00 trembles beneath you and opens up, swallowing your entire body leaving no trace of existence behind.
%teleport% %actor% 19816
%force% %actor% look
%echo% %actor.name% vanishes beneath the ground.
end
~
#19823
Lion load trigger~
0 d 100
reload~
if %speech%==reload
%echo% the Sleeping &03Lion&00 has arrived and prepares for his next match.
%load% mob 19807
end
~
#19824
lion rewield trigger~
2 d 100
rewield~
if %speech%==rewield
%load% object 37802
wait 1
%force% lion get short
wait 2
%force% lion say Who left my sword lying in the dirt?!?
%force% lion wield short
end
%force% lion say Prepare to feel cold steel my friend!
end
~
#19825
lion wield trigger~
0 k 100
~
context %self.id%
if (%already_fighting%)
wait 10
wield short
end
~
#19826
Genesis transport trigger~
2 d 100
gs~
if %actor.clan% != 14
%send% %actor% You are not worthy enough to enter the book.
%damage% %actor% 100
%teleport% %actor% 3030
halt
end
%teleport% %actor% 19828
%force% %actor% look
%echo% %actor.name% is slowly absorbed into the large book, and disappears. 
end
~
$~
