#9000
test3~
0 b 100
~
shout help me!^M
belch^M
~
#9001
DwarfLover~
0 g 100
~
if %actor.is_npc% !=0
halt
end
if %actor.race% == dwarf
say I pray the dwarves live long even in my folly.
else
say May the death of you restore my clans honor!
end
~
#9002
chaoswield~
1 j 100
~
if %actor.vnum% == -1
if %actor.level% >= 45
 %echo% You hear. 'So you think you are powerful enough to control me?'
else
 %echo% You hear. 'Hahaha foolish mortal, you think you can control the chaos?'
 return 0
end 
end
~
#9003
removechaos~
1 l 100
~
%echo% You hear, 'What's the matter, not man enough to wield me?'
end
~
#9004
givetouch~
1 i 100
~
%echo% You Hear, "Another puny mortal soul for my collection!
end
~
#9005
fightweapon~
1 b 100
~
 
*Narg is percent chance trigger will fire every 13 seconds
*arguements no used for this type of trigger
*look at trig 9008 that evals the wearer to variable chaos, makes it 
*global so it can be used with other trigs, there are no preset
*variables to this object trigger so you have to make one up
~
#9006
dropchaos~
1 gh 100
~
%echo% You hear, "Death to the infidels who dare disobey me."
end
~
#9007
chaos1~
1 c 100
e ea eas east~
dg_script cast 'earthquake'
end
~
#9008
touchofchaoswield~
1 j 100
~
if %actor.is_npc% !=0
halt
else
eval chaos %actor%
global chaos
end
~
#9009
randomattack~
1 b 100
~
eval target %random.char%
if %target.level% >51
halt
else
dg_cast 'doom bolt' %char%
end
~
#9010
Berserk trigger~
0 k 20
~
wait %random.8%
berserk
~
#9011
Instant Poison mob trigger~
0 e 0
is off balance and vulnerable!~
eval success %random.2%
if %success% < 2
halt
end
say Ahah, I have you right where I want you!
wait 3
poison %actor.name%
wait 1
say Take that!
~
$~
