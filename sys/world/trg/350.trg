#35029
Cavern O Hero's Portal Generator~
1 an 100
~
eval portalset %random.3%
if %portalset% == 1
%echo% this is set one
%load% obj 35066
%load% obj 35070
%load% obj 35081
elseif %portalset% == 2
%echo% this is set two
%load% obj 35067
%load% obj 35071
%load% obj 35075
elseif %portalset% == 3
%echo% this is set three
%load% obj 35068
%load% obj 35072
%load% obj 35088
end
~
#35030
mertis mirror load~
0 d 1
Castle Mertis~
if %speech%==Castle Mertis
wait 1
%echo% &07A thick fog begins to fill the room.&00
mload obj 35088
wait 10
%echo% The young heroin grins insanely and runs into the mist
%purge% self
end
~
#35031
coh mirror load~
0 d 1
Cavern of Heroes~
if %speech%==Cavern of Heroes
wait 1
%echo% A large portal appears in the center of the room.
wait 10
mload obj 35081
%echo% The young heroin throws her bow over her shoulder and dives into the portal
%purge% self
end
~
#35032
heroin speek~
0 g 100
~
%echo% A young heroin screams,'Halt, who goes there?!'
wait 20
say Ahhh, a fellow adventurer, glad to meet your acquaintance. 
wait 20
say Alas, my torment has come to an end.
wait 5
relax
wait 20
say Until another adventurer comes to take my place i was forced to watch over this place.
wait 20
say May i ask which area you intend to explore?
wait 20
say Whichever you choose i will go ahead to make sure it's safe.
wait 20
say You may either go to Castle Mertis or Cavern of Heroes.
wait 20
say What say you!
wait 5
pace
end
~
$~
