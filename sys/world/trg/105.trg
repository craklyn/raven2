#10501
werekitty~
0 k 80
~
%echo% The bunny rabbit hisses and raises its ears and sprouts fangs!
mtransform 10581
wait 150
%echo% The were-bunny sooths itself and it's fangs receed.
mtransform 10582
~
#10502
lightning~
0 g 100
~
if %actor.is_npc% == 0 && %actor.name% && %actor.level% < 51
   dg_cast 'lightning' %actor.name%
   end
~
#10503
aggrohuman~
0 g 100
~
if %actor.is_npc% != 0
  halt
end 
if %actor.race% == human
  say hi
end 
~
#10550
new trigger~
0 g 100
~
say The key to this door lies in one of the houses of this city!  Find it before its too late!
~
#10551
FollowTheLeader~
0 d 100
follow me~
fol %actor.name%
~
#10552
MyHero!~
0 g 100
~
shou My heros!
say However can I get home?  I do not know the way!
~
#10553
fistonenter~
0 g 100
~
if (%actor.is_npc% !=0 || %actor.level% >50)
halt
else
stand
fist %actor.name%
end
~
#10598
new trigger~
0 g 100
~
sa Walk you lazy bastage.
~
$~
