#1420
Fortune Cookie Trigger~
1 c 3
crush~
eval cookie %random.21%
if %cookie% == 1
%echoaround% %actor% %actor.name% greedily crushes %self.shortdesc% and nothing but crumbs fall to the ground.  It's a dud!!
%send% %actor% You greedily crush %self.shortdesc% and crumbs fall to the ground, but it contains nothing else.  It's a dud!
end
if %cookie% > 1
%echoaround% %actor% %actor.name% greedily crushes %self.shortdesc% into crumbs.  A note from inside falls to the ground.
%send% %actor% You greedily crush %self.shortdesc% into crumbs and a note contained within falls to the ground.
end
%load% obj 1421
oforce %actor% get 1.
if %cookie% == 2
%load% obj 1422
elseif %cookie% == 3
%load% obj 1423
elseif %cookie% == 4
%load% obj 1424
elseif %cookie% == 5
%load% obj 1425
elseif %cookie% == 6
%load% obj 1426
elseif %cookie% == 7
%load% obj 1427
elseif %cookie% == 8
%load% obj 1428
elseif %cookie% == 9
%load% obj 1429
elseif %cookie% == 10
%load% obj 1430
elseif %cookie% == 11
%load% obj 1431
elseif %cookie% == 12
%load% obj 1432
elseif %cookie% == 13
%load% obj 1433
elseif %cookie% == 14
%load% obj 1434
elseif %cookie% == 15
%load% obj 1435
elseif %cookie% == 16
%load% obj 1436
elseif %cookie% == 17
%load% obj 1437
elseif %cookie% == 18
%load% obj 1438
elseif %cookie% == 19
%load% obj 1439
elseif %cookie% == 20
%load% obj 1440
elseif %cookie% == 21
%load% obj 1441
end
if %cookie% > 1
oforce %actor% get 1.
end
%purge% %self%
~
#1421
Sun Zhu Master of Terrain Trigger~
0 k 6
~
say &09I dare ask, if the enemy is numerous, disciplined and about to advance, how should one respond?&00
say &09I am the master of all nine forms of terrain.  Have you mastered even this one?&00
say &08WE SHALL SEE!&00
wait 5
say Earth swallow thee!
dg_cast 'earthquake'
dg_cast 'earthquake'
dg_cast 'earthquake'
dg_cast 'tremor'
~
#1422
Sun Tzu Water Strategy~
0 k 7
~
say &11Now the Army's disposition of force is like water. &00
say &11Water's configuration avoids heights and races downward.&00
say &08Allow me to demonstrate!&00
wait 15
dg_cast 'tsunami'
dg_cast 'tsunami'
~
#1423
Sun Tzu Arrow trigger~
0 k 0
~
*   %echo% actor is %actor.name%
*set room_var %actor.room%
*%echo% Room variable is %room_var%
*set enemies %people.room_var%
*%echo% Enemies variable is %enemies%
*shout Loose the catapults on these infidels!!
*wait 3 s
*%echo% Fireballs fall from the heavens!
**Next, shoot fireballs relative to how many people are in the room.
*%echo% while %enemies%
*set target %random.char%
*%echo% random.char is %random.char%
*dg_cast 'fireball' %target.name%
*eval enemies = enemies - 1
*%echo% enemies reduced in value is %enemies%
*%echo% done/s 
~
#1424
Sun Tzu Banter~
0 b 7
~
if %phrase% == 1
say In general, commanding a large number is like commanding a few.  it is a question of dividing up the numbers.
say Fighting with a large number is like fighting with a few.  It is a question of configuration and designation.
elseif %phrase% == 2
say One who excels at employing the military cultivates the Tao and preserves the laws
therefore, he is able to be the regulator of victory and defeat.
elseif %phrase% == 3
say If before the mission has begun it has already been exposed, the spy and those he informed should all be put to death.
elseif %phrase% == 4
say Advance knowledge cannot be gained from ghosts and spirits, inferred from 
say phenomena, or projected from the say measures of Heaven, but must be gained 
say from men for it is the knowledge of the enemy's true situation.
elseif %phrase% == 5
say Thus one who excels at employing the army leads them by the 
say hand as if they were only one man, so they cannot avoid it.
elseif %phrase% == 6
say When the soldiers and officers have penetrated deeply into enemy territory, 
say they will cling together.  When there is no alternative, they will fight.
elseif %phrase% == 7 
say Assembling the masses of the Three Armies, casting them into danger, is the responsibility of the general.
elseif %phrase% == 8
say Now if someone is victorious in battle and succeeds in attack but 
say does not exploit the achievement, it is disastrous, and his fate should 
say be termed 'wasteful and tarrying.'  Thus it is said the wise general
say ponders it, the good general cultivates it.
elseif %phrase% == 9
say If the enemy opens the door, you must race in!
elseif %phrase% == 10
say If the enemy is numerous and disciplined, how should we respond?
say I would say, first seize something that they love for then they
say will listen to you.
elseif %phrase% == 11
say Thus if you command with the civil and unify them through the 
say martial, this is what is referred to as 'being certain to take them'.
else
say Do not intercept well-ordered flags; do not attack well-regulated 
say formations.  This is the way to control changes.
end
~
#1425
Sun Tzu Antitank Trigger~
0 k 12
~
shout &09Let loose the volley &08on mine enemy&09!!&00
say My enemy is %actor.name%.
wait 10
%echo% A tremendous volley of arrows rains down from the heavens.
%damage% %actor% %random.650%
%echoaround% %actor% Several arrows sprout from %victim.name%'s chest.
%send% %actor% You take several arrows in the chest.
~
#1426
Sun Tzu Paralysis Tank Trigger~
0 k 12
~
say I can see you and your every weakness.
   say Allow me but a moment to study your techniques!
   wait 10
   %echoaround% %actor% Sun Tzu reaches out and touches the shoulder blades, temple, and inner thigh of %actor.name%.
   %send% %actor% Sun Tzu reaches out and touches your shoulder blades, temple, and inner thigh.
   dg_cast 'paralyze' %actor.name%
   dg_cast 'paralyze' %actor.name%
   dg_cast 'paralyze' %actor.name%
   dg_cast 'paralyze' %actor.name%
   dg_cast 'paralyze' %actor.name%
~
#1427
Sun Tzu Anti Support~
0 e 1
qpabraw maaw yufovirr canduszuzo~
if %actor.level% > 50
   halt
   end   
   eval success %random.2%
   if %success% == 2
   halt
   end
   say Oh, so that's how you are, eh?  You rely on magic to outdo me?
   say I will bring an end to this!  Let me only find a weakness in your form.
   %echoaround% %actor% Sun Zhu looks piercingly at %actor.name%.
   %send% %actor% Sun Zhu looks piercingly at you.
   wait 5
   %echoaround% %actor% Sun Tzu reaches out and touches the shoulder blades, temple, and inner thigh of %actor.name%
   %send% %actor% Sun Tzu reaches out and touches your shoulder blades, temple, and inner thigh.
   dg_script 'paralyze' %actor.name%
   dg_script 'paralyze' %actor.name%
   dg_script 'paralyze' %actor.name%
   dg_script 'paralyze' %actor.name%
   dg_script 'paralyze' %actor.name%
~
#1428
Fortune Cookie Maker~
0 f 100
~
%load% obj 1420
%echo% As Sun Tzu drops to the ground, a light flashes from within his robes. 
First part gave a fortune cookie.  Second part gives quest points.
   establish the group master
this will escalate past ungrouped charmies
eval master %actor% 
eval more 1
while %more%
if %master.master% && (%master.is_npc% || %master.is_grouped%)
eval master %master.master%
else
eval more 0
end
done
reward the master
if %master.is_npc% == 0 && %master.room% == %actor.room% && %master.is_grouped%
%send% %master% &13You have earned 2 qp!&00
nop %master.qp(2)%
end
step through the followers to reward them
eval follower 0
meta ch master.follower(%follower%)
while %ch%
   if %ch.is_npc% == 0 && %ch.room% == %actor.room% && %ch.is_grouped%
%send% %ch% &13You have earned 2 qp!&00
nop %ch.qp(2)%
end
eval follower %follower% + 1
meta ch master.follower(%follower%)
done
return 0
~
#1429
Antiduster~
0 e 0
You swiftly block~
   eval success %random.100%
   if %success% < 20
   halt
   end
   say Oh, so that's how you are, eh?  You rely on mere parlor tricks to get the best of me?
   say I will bring an end to this!  Let me only find a weakness in your form.
   %echoaround% %actor% Sun Zhu looks piercingly at %actor.name%.
   %send% %actor% Sun Zhu looks piercingly at you.
   wait 20
   if %actor.level% > 50
   halt
   end
   %echoaround% %actor% Sun Tzu reaches out and touches the shoulder blades, temple, and inner thigh of %actor.name%
   %send% %actor% Sun Tzu reaches out and touches your shoulder blades, temple, and inner thigh.
   dg_cast 'paralyze' %actor.name%
   dg_cast 'paralyze' %actor.name%
   dg_cast 'paralyze' %actor.name%
   dg_cast 'paralyze' %actor.name%
~
#1435
New years guy~
0 g 15
~
wait 2 s
drink fountain
say Hello to you kind sir or madam!  I am a simply wanderer, could you spare me a bite to eat?
~
#1436
bread trigger~
0 j 100
~
say Thank you for sharing your wealth with me, kind one.
say May it be a gift to the gods.
say And in return, here's a little something for your way.
%load% obj 1454
give champagne %actor.name%
wink %actor.name%
wait 2 s
junk %object.name%
~
#1437
Eat trigger~
1 cfg 0
~
say My trigger commandlist is not complete!
~
#1456
Malevolent Pinata Trigger~
2 c 100
Smash~
eval cookie %random.18%
if %actor.has_item(stick)%
%send% %actor% You don't seem to have a stick!
halt
end
if %cookie% < 10
%echoaround% %actor% %actor.name% greedily crushes Malevolent and nothing but crumbs fall to the ground.  It's a dud!!
%send% %actor% You greedily crush Malevolent but only crumbs fall to the ground.  It's a dud!
end
if %cookie% >= 10
%echoaround% %actor% %actor.name% greedily crushes Malevolent.  Something falls to the ground.
%send% %actor% You greedily crush Malevolent into crumbs and something falls to the ground.
end
if %cookie% == 10
%load% obj 1412
elseif %cookie% == 11
%load% obj 1650
elseif %cookie% == 12
%load% obj 1655
elseif %cookie% == 13
%load% obj 1691
elseif %cookie% == 14
%load% obj 1692
elseif %cookie% == 15
%load% obj 1693
elseif %cookie% == 16
%load% obj 1694
elseif %cookie% == 17
%load% obj 24401
elseif %cookie% == 18
%load% obj 33185
end
if %cookie% >= 10
oforce %actor% get 1.
end
~
#1480
Slash's growl~
0 b 100
~
eval target %random.char%
%echo% %target.name%
if %target.is_pc%
halt
else
backstab %target.name%
end
~
#1481
Greased pig flee trigger~
0 k 50
~
open door
flee
~
#1482
pig teleport~
0 k 60
~
eval target %random.char%
if %target.is_npc% || %target.level% >50
halt
else
if %target%
%teleport% %target% 18001
force %target% look
mat 18001 %echo% %target.name% slides in covered in grease and pig dung.
end
~
#1483
slashs greet trigger~
0 g 100
~
if %actor.name% == Frogalog || %actor.name% == Elminster
%echo% Slash purrs contentedly.
halt
else
backstab %actor.name%
end
~
#1484
binky~
0 j 100
~
if %object.name% == binky
junk binky
mtransform -1485
%echo%   Fools! you have fallen into my masters trap!
mhunt %actor%
else
%echo% The baby new year smiles happily and fills its diaper.
end
~
#1485
binky follow~
0 j 100
~
if %object.name% == binky
follow %actor.name%
else
end
~
#1486
dargoth~
0 f 100
~
%echo% %actor% grabs a small grey object from Dargoths hands as he vanishes.
%purge% corpse
~
#1487
puppy ~
0 g 100
~
eval name  %actor.name%
eval race %actor.race%
if %name% == Radagast
%echoaround% %actor.name% A puppy enthusiastically begins humping %actor.name%'s leg.
%send% %actor.name% A puppy looks up at you with big round eyes.
halt
elseif %race% == Werewolf
%echo% The puppy sees you and barks playfully.
end
if %name% == Frogalog
follow Frogalog
%send% %name% A puppy looks at you lovingly.
bark
end
~
#1488
elms remote~
2 d 100
1 2 3~
eval result %speech%
%echo% result
~
#1489
decon station trigger~
2 c 100
push~
%echoaround% %actor% %actor.name% enters the decontamination shower.
%send% %actor% A needle sharp spray of blistering hot water strikes you.
%force% %actor% noteam
wait 1 s
%send% %actor% The shower ends, and you enter the dryer.
%echoaround% %actor% %actor.name% steps under the dryer.
dg_cast 'cleanse' %actor%
wait 1 s
%send% %actor% you feel refreshed and ready for more action.
%echoaround% %actor% %actor.name% fades from this reality.
dg_cast 'revive' %actor%
%teleport% %actor% 18001
%force% %actor% look
~
#1490
apple tree hasana game~
2 b 10
~
eval phrase %random.5%
   if %phrase% ==1
     %echo% A large red apple drops to the ground and rolls away.
   elseif %phrase% ==2
     %echo% A light breeze rustles the leaves of the apple tree.
   elseif %phrase% ==3
%echo% A squirrel jumps from one branch to the next.
   elseif %phrase% ==4
     %echo% A thud is heard as an apple drops from the tree..
   else
     %echo% Lovely fragrances of ripe apples linger in the air.
   end
~
#1491
apple game hasana cider~
0 j 100
apple~
if %object.vnum% == 1491
wait 1 s
say That should make a nice cup'o cider.
wait 2 s
eval chance %random.3%
switch %chance%
case 1
%load% obj 1492
give cider %actor.name%
%send% %actor.name% You have earned 1 qp!
nop %actor.qp(1)%
mload mob 1452
mgoto 36549
mpurg self
break
case 2
%load% obj 1493
give cider %actor.name%
%send% %actor.name% You have earned 1 qp!
nop %actor.qp(1)%
mload mob 1452
mgoto 36549
mpurg self
break
default
%load% obj 1494
give cider %actor.name%
%send% %actor.name% You have earned 1 qp!
nop %actor.qp(1)%
mload mob 1452
mgoto 36549
mpurg self
else
wait 1 s
say I don't think this will make good cider.
wait 1 s
drop all
endif
~
#1493
aggrivator hasana random~
0 g 100
~
eval phrase %random.31%
if %phrase% == 1
say Lets play house. You be the door and I'll slam you.
flee
elseif %phrase% == 2
say We all spring from apes, but you didn't spring far enough.
flee
elseif %phrase% == 3
say You have a good family tree, but the crop is a failure.
flee
elseif %phrase% == 4
say We know you could not live without us. We'll pay for the funeral.
flee
elseif %phrase% == 5
say In the dictionary under the word, "stupid," it says, "see him."
point %actor.name%
flee
elseif %phrase% == 6
say You're the best at all you do, and all you do is make people hate you.
flee
elseif %phrase% == 7
pet %actor.name%
flee
elseif %phrase% == 8
loser %actor.name%
flee
elseif %phrase% == 9
say We know that romance brings out the beast in you -- the jackass.
flee
elseif %phrase% == 10
say Is your name Maple Syrup? It should be, you sap.
flee
elseif %phrase% == 11
say Man alive! But I wish you weren't.
flee
elseif %phrase% == 12
say There was something about you that I liked, but you spent it.
flee
elseif %phrase% == 13
say Sit down and give your mind a rest.
flee
elseif %phrase% == 14
burp
flee
elseif %phrase% == 15
say We can always tell when you are lying. Your lips move.
flee
elseif %phrase% == 16
slam %actor.name%
flee
elseif %phrase% == 17
say When you talk, other people get hoarse just listening.
flee
elseif %phrase% == 18
say I thought of you all day today when I was at the zoo.
flee
elseif %phrase% == 19
say If ignorance is bliss, then why aren't you people happy?
flee
elseif %phrase% == 20
say If you can't beat them, arrange to have them beaten.
flee
elseif %phrase% == 21
boggle %actor.name%
flee
elseif %phrase% == 22
lick %actor.name%
flee
elseif %phrase% == 23
strut
flee
elseif %phrase% == 24
say Out of my mind. Back in five minutes.
flee
elseif %phrase% == 25
say He who laughs last thinks slowest.
flee
elseif %phrase% == 26
say Reality? That's where the pizza delivery guy comes from.
flee
elseif %phrase% == 27
taunt %actor.name%
flee
elseif %phrase% == 28
say The gene pool here could use a little chlorine.
flee
elseif %phrase% == 29
say Cover me. I'm changing lanes.
flee
elseif %phrase% == 30
say Rat: The other white meat!
point %actor.name%
flee
else
poke %actor.name%
flee
end
~
#1494
pester escape trigger hasana game~
0 k 30
~
if %actor.is_npc% || %actor.level% >50
halt
else
%teleport% %actor% 18001
mat 18001 %echo% %actor.name% does a face plant in the dirt.
end
~
#1496
shimmering cube loader~
0 f 100
~
%load% obj 18198
   %echo% As the guardian drops to the ground, a light flashes from within his cloak. 
   return 0
~
$~
