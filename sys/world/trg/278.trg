#27800
Brutus welcome~
0 gh 100
~
if %direction%
wait 1
say Hello, %actor.name%, how are things to the %direction%?
wait 2
wait 1
say Please be sure to clean up after yourself.
wait 2
wait 1
say Have a pleasent journey, %actor.name%.
end
~
#27801
Singing Icingdeath~
1 b 60
~
eval phrase %random.5%
if %phrase% == 1
%echo% %self.shortdesc% sings, 'Rooooxxxxxaaannne, you don't have to put on the &01red&00 light
elseif %phrase% == 2
%echo% %self.shortdesc% says, 'When do I get to meet the girlfriend?'
elseif %phrase% == 3
%echo% %self.shortdesc% says, 'This is boring, lets go kill something'
elseif %phrase% == 4
%echo% %self.shortdesc% screams 'BANZAI!!! and brandishes itself for battle
elseif %phrase% == 5
%echo% %self.shortdesc% says, 'Kiss me, oh wait...I'm just a %self.shortdesc%
end
~
#27802
salamander king teleport~
2 c 100
s~
if %cmd.mudcommand% == say && %arg% == igne natura renovatur integra
%echo% A &06black&00 cloud of smoke begins to swirl around %actor.name%
wait 20
%echo% %actor.name% screams violently then disappears.
wait 20
%teleport% %actor% 27840
%force% %actor% look
else
return 0
end
~
#27803
MN mob load zone~
2 f 100
~
Nothing.
~
#27804
Brutis load eq~
0 n 100
~
Nothing.
~
#27805
MN portal orb enter~
2 f 100
~
%load% obj 27820
~
#27806
mini boss teleport 50%~
0 l 50
~
eval %self.id%
%echo% %self.name% &01stops attacking and begins to slowly fade out of existence.&00
mgoto 27858
detach 27806 %self.id%
end
~
#27807
aggrodjinn~
0 i 100
~
if %actor.is_npc% ! = 0 || %actor.level% > 50
halt
else
dg_cast 'sandstorm' %actor.name%
end
~
#27808
mini boss teleport 25%~
0 l 25
~
eval %self.id%
%echo% %self.name% &01stops attacking and begins to slowly fade out of existence.&00
mgoto 27859
detach 27808 %self.id%
end
~
#27809
mn chest load~
0 n 100
~
get gold
unlock chest
open chest
put 100000 coin chest
close chest
lock chest
junk key
%purge% abracadabra
~
#27810
water hole load~
2 f 100
~
Nothing.
~
#27811
mini boss teleport 75%~
0 l 75
~
eval %self.id%
%echo% %self.name% &01stops attacking and begins to slowly fade out of existence.&00
mgoto 27857
detach 27811 %self.id%
end
~
#27812
mother nature blink/load~
2 c 100
s~
if %cmd.mudcommand% == say && %arg% == I seek Terra Mater
%echo% A small - &10bright&00 light appears from above, descending down upon you.
wait 10
%echo% The light suddenly &08explodes&00, releasing someone... or something... into the room.
wait 15
%echo% You hear tiny footsteps racing across the wooden floor.
wait 20
%load% mob 27808
else
return 0
detach 27812 %self.id%
end
~
#27813
scuttle close/locl~
2 g 100
~
say My trigger commandlist is not complete!
~
#27814
key load abracadabra chest~
0 n 100
~
%load% obj 27816
~
#27815
Salamandridae gear load~
0 n 100
~
Nothing.
~
#27816
Salamandridae load room~
2 f 100
~
Nothing.
~
#27817
mn pull rope~
2 c 100
p~
if %cmd% == pull && %arg% == rope
%echo% %actor.name% stands in awe as a bright &25&10pulsating light&00 appears from within the tree - nearing him.
wait 4
%echo% %actor.name% drops to his knees.
wait 5
%echo% %actor.name% vanishes in thin air.
%teleport% %actor% 27818
%force% %actor% look
else
return 0
end
~
#27818
larva drain~
0 k 100
~
eval myname %self.name%
eval draintarget 28.char%
if (%draintarget.name% != %myname%)
wait 3
dg_cast 'energy drain' %draintarget%
end
~
#27819
khal teleport/load larva/80%~
0 l 80
~
eval %self.id%
%echo% %self.name% bellows, '&01I'M BORED, LET MY SERVANTS FINISH YOU!&00
wait 5
%echo% From the &08fires&00 below rises a &05golem&00.
%echo% From the &08fires&00 below rises a &05golem&00.
%echo% From the &08fires&00 below rises a &05golem&00.
%load% mob 27834
%load% mob 27834
%load% mob 27834
mgoto 27871
detach 27819 %self.id%
end
~
#27820
khal teleport/mob load/60%~
0 l 60
~
Nothing.
eval %self.id%
%echo% %self.name% bellows, '&01NOW I'M GETTING IRRITATED!&00
%echo% From the &08fires&00 below rises a &05golem&00.
%echo% From the &08fires&00 below rises a &05golem&00.
%echo% From the &08fires&00 below rises a &05golem&00.
%echo% From the &08fires&00 below rises a &05golem&00.
%echo% From the &08fires&00 below rises a &05golem&00.
%load% mob 27834
%load% mob 27834
%load% mob 27834
%load% mob 27834
%load% mob 27834
mgoto 27881
detach 27820 %self.id%
end
~
#27821
khal teleport/larva load/40%~
0 l 40
~
eval %self.id%
%echo% %self.name% bellows, '&01I'M BORED AGAIN FOR YOU FOOLS ARE PUNY, PERHAPS I NEED MORE MINIONS&00
wait 5
%echo% From the &08fires&00 below rises a &05golem&00.
%echo% From the &08fires&00 below rises a &05golem&00.
%echo% From the &08fires&00 below rises a &05golem&00.
%echo% From the &08fires&00 below rises a &05golem&00.
%echo% From the &08fires&00 below rises a &05golem&00.
%echo% From the &08fires&00 below rises a &05golem&00.
%echo% From the &08fires&00 below rises a &05golem&00.
%echo% From the &08fires&00 below rises a &05golem&00.
%load% mob 27834
%load% mob 27834
%load% mob 27834
%load% mob 27834
%load% mob 27834
%load% mob 27834
%load% mob 27834
%load% mob 27834
mgoto 27883
detach 27821 %self.id%
end
~
#27822
khal 35% kheli load/assist~
0 l 35
~
eval %self.id%
%echo% %self.name% bellows, &01NEVER SEND A MINION TO DO ITS MASTERS JOB&00
%echo% %self.name% bellows, '&01PRINCESS, COME AID ME IN MY VICTORY!&00'
%echo% &10Kheli&00 &06emerges out of the mist&00
%load% mob 27835
detach 27822 %self.id%
~
#27823
Kheli gear load~
0 n 100
~
mload obj 27876
wear anklet
mload obj 27877
wear robe
end
~
#27824
Ayame charm summons 75%~
0 l 75
~
eval %self.id%
%echo% %self.name% says, "&25&08Come my Cobras, aid me in battle!&00"
%load% mob 27836
%load% mob 27836
%load% mob 27836
detach 27824 %self.id%
~
#27825
Ayame charm summons 50%~
0 l 50
~
eval %self.id%
%echo% %self.name% says, "&25&08The Cobras were just a start, let my Pythons devour you!&00"
%load% mob 27837
%load% mob 27837
%load% mob 27837
detach 27825 %self.id%
~
#27826
Ayame Anaconda load 25%~
0 l 25
~
eval %self.id%
%echo% %self.name% says, "&25&08Perhaps you're stronger than I thought, let the Anaconda crush you!&00"
%load% mob 27838
%load% mob 27838
%load% mob 27838
detach 27826 %self.id%
~
#27827
Ayame blood room~
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
%echoaround% %victim% &08blood&00 starts to seep through %victim.name%'s clothes.
%send% %victim.name% &07You start to feel tired from the lack of&00&08 blood&00.
else
halt
~
#27828
Bacara spell list cast~
0 k 75
~
eval spellcast %random.3%
switch %spellcast%
case 1
wait 3
dg_cast 'sand storm'
break
case 2
wait 3
cast 'paralyze' %random.char%
break
case 3
wait 3
cast 'wall of fire'
break
done
~
#27829
testing calm force kill~
0 e 100
calming~
say &08NOBODY CAN STOP ME, NOT NOW...NOT EVER!!!&00
kill bobby
~
#27830
death tickle spell~
0 e 100
tickles you~
pond
say To &08kill&00?
wait 3 sec
say Or to let &11live&00?
wait 3 sec
pond
wait 3 sec
say You look pitiful, %actor.name%.  I shall spare you...this time.
wait 4 sec
say Perhaps these spells will help you become something of yourself, instead of the pitiful rat that you are.
wait 4 sec
dg_cast 'haste' %actor%
dg_cast 'flame blade' %actor%
dg_cast 'sanctuary' %actor%
wait 3 sec
say Now, be gone with your trash.
end
~
#27831
Cassiel wizslap~
0 e 100
slapped~
growl
say &01HOW DARE YOU SLAP AN ANGEL!&00
wait 3 sec
say &01BURN YOU FOOL!&00
dg_cast 'teleport' %actor%
end
~
#27832
tom turkey halo~
0 m 1000000
~
say Gobble gobble, %actor.name%
wait 5 sec
say Gobble, gobble...hmm...gobble?
wait 3 sec
chuck
wait 3 sec
say GOBBLE!
flex
%load% obj 27899
drop feather
%force% %actor% get feather
say Gobble....gobble gobble!
bow %actor%
~
$~
