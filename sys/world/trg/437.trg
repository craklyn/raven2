#43700
mia tele trigger 2~
0 dg 1
I agree~
if %speech%==I agree
wait 1
Say Thank you ever so much, %actor.name%
wait 2
%teleport% %actor% 43700
%force% %actor% look
end
~
#43701
teleport trigger for mob entry into castle mertis~
0 h 100
'I agree'~
Say Thank god you've finally found me. I've been lost and stumbling around in this land unknown to me for sometime now.
wait 1
Say Would you be so kind as to help me rescue my Mother and Slay the tyrant who keeps her locked up?
wait 2
Say Oh please help me!
wait 3
Say If you agree, please say "I agree" and I will send you there with what energy i have left.
wait 4
end
~
#43703
mia final trig~
0 dg 100
~
say My trigger commandlist is not complete!
~
#43713
shingars aggro trigg.~
0 h 100
~
eval class %self.class%
eval level %self.level%
eval align %self.align%
eval victim %actor%
if %victim.is_npc% != 0 || %victim.level% > 50
%send% %self% %victim.name% is an immortal, or NPC
halt
else
%send% %self% %victim.name% is a valid target
switch %class%
case Warrior
* Do what I do best
if %self.eq(19)%
bash %actor.name%
else
kill %actor.name% 
kick %actor.name%
end
      break
case Ranger
* Do what I do best
trip %actor.name%
break
case Shou-Lin
* Do what I do best
if %level% < 12
kill %actor.name%
elseif %level% >= 12 && %level% < 18
sweep %actor.name%
elseif %level% >= 18 && %level% < 23
blind %actor.name%
elseif %level% >= 23 && %level% < 33
knock %actor.name%
      else
fist %actor.name%
end
break
case Cleric
* Do what I do best
if %level% >= 43 && %align% < -250
dg_cast 'unholy word'
      elseif %level% >= 43 && %align% > 250
dg_cast 'holy word'
else 
dg_cast 'earthquake'
end
break
case Solamnic Knight
* Do what I do best
if %level% < 9
kill %actor.name%
elseif %level% >= 9 && %level% < 44 && %self.eq(19)%
bash %actor.name%
elseif %level% > 44 && %actor.align% < -250
dg_cast 'holy word'
else 
        kick %actor.name%
end
break
case Death Knight
* Do what I do best
if %level% < 9
kill %actor.name%
elseif %level% >= 9 && %level% < 30 && %self.eq(19)%
bash %actor.name%
elseif %level% >= 30 && $level% < 45
dg_cast 'pestilence'
elseif %level% >= 45 && %actor.align% > 250
dg_cast 'unholy word'
else 
kick %actor.name%
      end
break
case Shadowdancer
* Do what I do best
if %level% < 10 && %self.eq(19)%
backstab %actor.name%
elseif %level% >=10 && %level% < 40
dg_cast 'shadow blades' %actor.name%
elseif %level% >= 40
dg_cast 'shadow dance' %actor.name% 
else
kill %actor.name%
end
break
case Assassin
* Do what I do best
if %self.eq(19)%
back %actor.name%
else 
dust %actor.name%
end
break
case Thief
        * Do what I do best
if %self.eq(19)%
back %actor.name%
else 
dust %actor.name%
end
break    
case Magic User
* Do what I do best
      if %level% < 6
kill %actor.name%
elseif %level% > 6 && %level% < 25
dg_cast 'web' %actor.name%
elseif %level% >=25 && %level% < 30
dg_cast 'ice storm' 
elseif %level% >= 30 && %level% < 35
dg_cast 'sand storm' %actor.name%
elseif %level% >= 35 && %level% < 40
dg_cast 'lightning bolt' %actor.name%
elseif %level% >=40 && %level% < 45
dg_cast 'meteor swarm'
elseif %level% >= 45 && %level% < 50
dg_cast 'fireball' %actor.name%
else
dg_cast 'doom bolt' %actor.name%
end
break
default
        * Do what I do best
kill %actor.name%
break
else 
halt
end
roleplay Is that all you've got? *chuckle*
end
~
#43719
gorminis super aggro wrath trigger~
0 g 100
~
if %actor.is_npc% != 0 || %actor.level% > 50
halt
else
dg_cast 'wrath of the ancient' %actor.name%
end
~
#43736
dakus super aggro terror trigger~
0 g 100
~
if %actor.is_npc% != 0 || %actor.level% > 50
halt
else
dg_cast 'terror' %actor.name%
end
~
#43738
edjin hands of the wind trigger~
0 g 100
~
if %actor.is_npc% != 0 || %actor.level% > 50
halt
else
dg_cast 'hands of wind' %actor.name%
end
~
#43742
Sgerin super aggro calm,retarget soulpierce trig~
0 g 100
~
if %actor.is_npc% != 0 || %actor.level% > 50
halt
else
dg_cast 'soul pierce' %actor.name%
dg_cast 'soul pierce' %actor.name%
dg_cast 'soul pierce' %actor.name%
end
if %actor.is_npc% != 0 || %actor.level% > 50
halt
else
calm
calm
kill %actor.name%
cackle
Say I feel for you. This is going to hurt just a tad.
end
~
#43750
silirans aggro spell trigger~
0 g 100
~
if %actor.is_npc% != 0 || %actor.level% > 50
halt
else  
dg_cast 'word of recall' %actor.name%
agree self
Say no more Mr %actor.name%
end
if %actor.is_npc% != 0 || %actor.level% > 50
halt
else
say So you've all come to die then? Mwhahahaha.
dg_cast 'earthquake'
~
#43751
new trigger~
0 g 100
~
if %actor.is_npc% != 0 || %actor.level% > 50
halt
else
dg_cast 'dance of dream' %actor.name%
tell %actor.name% Having a nice sleep there? I'll gladly wake you.
end
backstab %actor.name%
end
~
#43755
executioners death touch trigger~
0 g 100
~
if %actor.is_npc% != 0 || %actor.level% > 50
halt
else
dg_cast 'death touch' %actor.name%
end
~
#43761
new trigger~
0 g 100
~
if %actor.is_npc% != 0 || %actor.level% > 50
halt
else
dg_cast 'dishearten' %actor.name%
use wand %actor.name%
end
if %actor.is_npc% != 0 || %actor.level% > 50
halt
else
dg_cast 'flame strike' %actor.name%
end
~
#43766
catheris backstab trig~
0 h 100
~
if %actor.is_npc% != 0 || %actor.level% > 50
halt
else
backstab %actor.name%
end
~
#43791
Edjin teleport trigger~
0 d 1
Take me to Elru~
if %speech%==Take me to elru
wait 1
%teleport% %actor% 43899
~
#43792
edjin walk in trig~
0 g 100
~
Say Have you come to challenge my master?
wait 1
Say If so only the correct phrase will allow me to teleport you to him.
end
~
#43793
artisans aggro gut trigg~
0 g 100
~
if %actor.is_npc% != 0 || %actor.level% > 50
halt
else
gut %actor.name%
end
~
#43798
slave keeper aggro~
0 g 100
~
if %actor.is_npc% != 0 || %actor.level% > 50
halt
else
bash %actor.name%
end
~
$~
