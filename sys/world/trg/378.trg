#37801
sparring partner load trig~
2 cd 100
reload~
if %speech%==reload
%echo% Your sparring partner has arrived.
%load% mob 19313
~
#37802
mob rewield trig~
2 cd 100
rewield~
if %speech%==rewield
%load% obj 37802
wait 1
%force% partner get short
wait 2
%force% partner say Who left my sword lying in the dirt?@!
%force% partner wield short
end
%force% partner say Prepare to feel cold steel!
end
~
#37803
wield trigger~
0 k 100
~
context %self.id%
if (%already_fighting%)
wait 10     
wield short
end
~
#37815
Clan teleport trigger~
2 c 100
k~
   if %cmd.mudcommand% == knock && 'twice /= '%arg% && %arg.strlen% > 0
     %send% %actor% You knock on the door twice.
     %send% %actor% Before you know it, you are in the President's suite!
%teleport% %actor% 37816
     %force% %actor% look
   else
     return 0
   end
~
#37820
Goblin Block~
0 cg 100
u~
if %actor.clan% != 22
  wait 10
  %send% %actor% The gluttonous goblin spits in your face!
  %echoaround% %actor% The gluttonous goblin spits in the face of &09%actor.name%&00.
  say &09Idiot!&00
  halt
end 
~
#37821
SMH clan teleport~
2 c 100
smd~
if %actor.clan% != 22
  %send% %actor% &07Only the chosen may enter the &14mines&00.
  %damage% %actor% 100
  %force% %actor% dismount
  %teleport% %actor% 3063
  %force% %actor% look
  halt
end 
%send% %actor% &07The ground opens and drags you down to the &14mines&00.
%force% %actor% dismount
%teleport% %actor% 37821
%force% %actor% look
~
#37825
SMH minion teleporter~
0 d 1
amara samsera new thalos under dark lost city elven freehold freeport turra abenos shaden midgaard help~
If %speech%==amara
  wait 1
  say &07I hope you have grown gills! &11Amara City&07 it is!&00
  wait 1
  %force% %actor% dismount
  %teleport% %actor% 35439
  %force% %actor% look
end
if %speech%==samsera
  wait 1
  say &07Samsera it is. Pillage all you can. Its for the cause.&00
  wait 1
  %force% %actor% dismount
  %teleport% %actor% 18001
  %force% %actor% look
end
if %speech%==turra
  wait 1
  say &07Turra will be your stop. Keep close your riches.&00
  wait 1
  %force% %actor% dismount
  %teleport% %actor% 27718
  %force% %actor% look
end
if %speech%==abenos
  wait 1
  say &07Abenos waits at end of the portal. No one can be trusted there.&00
  wait 1
  %force% %actor% dismount
  %teleport% %actor% 33138
  %force% %actor% look
end
if %speech%==freeport
  wait 1
  say &07The city of Freeport awaits. Do not travel alone.&00
  wait 1
  %force% %actor% dismount
  %teleport% %actor% 22953
  %force% %actor% look
end
if %speech%==elven
  wait 1
  say &07The destination city of Fairhaven is nice this time of year.&00
  wait 1
  %force% %actor% dismount
  %teleport% %actor% 30601
  %force% %actor% look
end
if %speech%==under dark
  wait 1
  say &07Tricksters and thugs await. The Under Dark is unforgiving.&00
  wait 1
  %force% %actor% dismount
  %teleport% %actor% 20684
  %force% %actor% look
end
if %speech%==Lost city
  wait 1
  say &07Do not lose yourself beyond the portal!&00
  wait 1
  %force% %actor% dismount
  %teleport% %actor% 21201
  %force% %actor% look
end
if %speech%==new thalos
  wait 1
  say &07Tell the Sultan to cram it with walnuts, ugly!&00
  wait 1
  %force% %actor% dismount
  %teleport% %actor% 13506
  %force% %actor% look
end
if %speech%==freehold
  wait 1
  say &07The portal will dump in you Freehold. Be safe!&00
  wait 1
  %force% %actor% dismount
  %teleport% %actor% 12223
  %force% %actor% look
end
if %speech%==shaden
  wait 1
  say &07Make sure your wares are protected. Shaden will swallow you whole.&00
  wait 1
  %force% %actor% dismount
  %teleport% %actor% 11501
  %force% %actor% look
end
if %speech%==midgaard
  wait 1
  say &07It is ok %actor.name%. We were all noobs once.&00
  wait 1 %force% %actor% dismount
  %teleport% %actor% 3005
  %force% %actor% look
end
if %speech%==help
  wait 1
  say Say the name of city youd like to visit. Your choices are: Amara City, Samsara, Midgaard, Turra, Abenos, Fairhaven (Elven), Under Dark, Lost City, Freehold, New Thalos, Freeport, and Shaden.
End
~
#37827
Peace Enter~
2 c 100
peace~
%send% %actor% &14Peace and quiet awaits.&00
%force% %actor% dismount
%teleport% %actor% 37829
%force% %actor% look
~
#37829
Hate Exit~
2 c 100
hate~
%send% %actor% &08The hate has grown too strong.&00
%force% %actor% dismount
%teleport% %actor% 37827
%force% %actor% look
~
$~
