#32401
eattrig~
0 g 100
~
get meal table
eat meal
belch
say That was good!
~
#32402
poettrigger~
0 g 100
~
say Hey bud, can you spare a minute?
~
#32403
noconvert~
0 c 100
conv~
say %actor.name%, I don't like preachy people.
kill %actor.name%
~
#32404
MagicWeapon~
1 bj 100
~
%echo% %self.shortdesc% glows hellishly.
%echo% %actor.fighting% is disfigured by %actor.name%'s blow.
~
#32405
GreetAttack~
0 g 100
~
if (%actor.is_npc% != 0 || %actor.level% > 50)
halt
else
stand
kill %actor.name%
end
~
#32406
GreetBash~
0 g 100
~
if (%actor.is_npc% != 0 || %actor.level% > 50)
halt
else
stand
bash %actor.name%
kick %actor.name%
end
~
#32410
unlockDoor~
2 d 100
"avoid the noid"~
wdoor 32408 north flags a
~
$~
