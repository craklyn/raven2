#27301
lightning trigger~
2 g 100
~
if %actor.is_npc% !=0
halt
else
dg_script cast 'lightning bolt'
cast 'lightning bolt'
end
~
#27302
gnarf ~
2 g 100
~
if %actor.is_npc% !=0
halt
else %echo% You hear high pitches voices talking in excited tones.
end
~
#27303
bruland~
0 g 100
~
if %actor.is_npc% !=0
halt
else
say 'May the thane forgive me, and may you die mortal scum!
end
~
#27304
gnarf2~
2 g 100
~
if %actor.is_npc% !=0
else
%echo% You hear a small child say 'mommy mommy, &23&25FRESH BLOOD! &00
end
~
#27306
Aggro~
0 g 100
~
if %actor.is_npc% != 0 || %actor.level% > 50
halt
end 
murd %actor.name%
~
#27307
back greeter~
0 g 100
~
back %actor.name%
~
#27308
holy word~
2 b 40
~
dg_cast 'holy word'
~
#27309
ice em!~
0 g 100
~
dg_cast 'ice storm'
~
#27310
chaos backer~
0 g 100
~
   if %actor.is_npc% != 0
halt
end 
back %actor.name%
~
$~
