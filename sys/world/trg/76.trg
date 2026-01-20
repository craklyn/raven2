#7601
Random Spell Presents~
1 g 100
~
eval trapnumber %random.9%
if %trapnumber% == 1
  %echoaround% %actor% %actor.name% opens the present.
  %echoaround% %actor% %actor.name% begins to shift in and out of reality.
  %send% %actor% You feel at home with the &07dead&00.
  dg_cast 'wraithform' %actor.name%
  dg_cast 'blur' %actor.name%
  return 0
  %purge% %self%
elseif %trapnumber% == 2
  %echoaround% %actor% %actor.name% opens the present.
  %echoaround% %actor% A small &13faerie&00 blinks in, and out, of the room.
  %send% %actor% A small &13faerie&00 blinks into the room, waves her wand at you, and vanishes.
  dg_cast 'armor' %actor.name%
  dg_cast 'bless' %actor.name%
  return 0
  %purge% %self%
elseif %trapnumber% == 3
  %echoaround% %actor% %actor.name% opens the present.
  %echoaround% %actor% A single &08red&00 snowflake floats down from above.
  %send% %actor% A single &08red&00 snowflake lands on the tip of your nose.
  dg_cast 'haste' %actor.name%
  dg_cast 'flame blade' %actor.name%
  return 0
  %purge% %self%
elseif %trapnumber% == 4
  %echoaround% %actor% %actor.name% opens the present.
  %echoaround% %actor% A jack-in-the-box SPRINGS out of the box!
  %send% %actor% A jack-in-the-box SPRINGS out of the box and laughs!
  %send% %actor% A jack-in-the-box says, 'Merry Christmas %actor.name%!'
  dg_cast 'sanctuary' %actor.name%
  dg_cast 'shield' %actor.name%
  return 0
  %purge% %self%
elseif %trapnumber% == 5
  %echoaround% %actor% %actor.name% opens the present.
  %echoaround% %actor% The sound of bells can be heard jingling high above.
  %send% %actor% You hear a faint "HO HO HO" and the jingling of bells.
  dg_cast 'pulse gain' %actor.name%
  dg_cast 'pulse heal' %actor.name%
  return 0
  %purge% %self%
elseif %trapnumber% == 6
  %echoaround% %actor% %actor.name% opens the present.
  %echoaround% %actor% %actor.name% has received the gift of knowledge and foresight!
  %send% %actor% You feel the sudden urge to learn.
  %send% %actor% You feel the sudden urge to learn even more.
  dg_cast 'fast learning' %actor.name%
  dg_cast 'true sight' %actor.name%
  return 0
  %purge% %self%
elseif %trapnumber% == 7
  %echoaround% %actor% %actor.name% opens the present.
  %echoaround% %actor% %actor.name% is surrounded by a veil of darkness.
  %send% %actor% You feel as if protected by the gods!
  dg_cast 'protection from evil' %actor.name%
  dg_cast 'protection from good' %actor.name%
  return 0
  %purge% %self%
elseif %trapnumber% == 8
  %echoaround% %actor% %actor.name% opens the present.
  %echoaround% %actor% %actor.name% is enveloped in &10l&14i&10g&14h&10t&00!
  %send% %actor% All darkness and evil seems to melt away.
  dg_cast 'holy armor' %actor.name%
  dg_cast 'divine mission' %actor.name%
  dg_cast 'righteous vision' %actor.name%
  return 0
  %purge% %self%
elseif %trapnumber% == 9
  %echoaround% %actor% %actor.name% opens the present.
  %echoaround% %actor% %actor.name% slips into the shadows.
  %send% %actor% You feel as one with the shadows.
  dg_cast 'shadow sphere' %actor.name%
  dg_cast 'shadow vision' %actor.name%
  dg_cast 'dance of shadows' %actor.name%
  return 0
  %purge% %self%
else 
  %echo% There doesn't appear to be anything inside the present. You wonder who took your gift.
  return 0 
end
~
#7602
Present Load/Slap~
2 f 100
~
%load% obj 9643
dg_cast 'teleport' present
%load% obj 9643
dg_cast 'teleport' present
%load% obj 9643
dg_cast 'teleport' present
%load% obj 9643
dg_cast 'teleport' present
%load% obj 9643
dg_cast 'teleport' present
%load% obj 9643
dg_cast 'teleport' present
%load% obj 9643
dg_cast 'teleport' present
%load% obj 9643
dg_cast 'teleport' present
%load% obj 9643
dg_cast 'teleport' present
%load% obj 9643
dg_cast 'teleport' present
%load% obj 9643
dg_cast 'teleport' present
%load% obj 9643
dg_cast 'teleport' present
%load% obj 9643
dg_cast 'teleport' present
%load% obj 9643
dg_cast 'teleport' present
%load% obj 9643
dg_cast 'teleport' present
%load% obj 9643
dg_cast 'teleport' present
%load% obj 9643
dg_cast 'teleport' present
%load% obj 9643
dg_cast 'teleport' present
%load% obj 9643
dg_cast 'teleport' present
%load% obj 9643
dg_cast 'teleport' present
%load% obj 9643
dg_cast 'teleport' present
%load% obj 9643
dg_cast 'teleport' present
%load% obj 9643
dg_cast 'teleport' present
%load% obj 9643
dg_cast 'teleport' present
%load% obj 9643
dg_cast 'teleport' present
%load% obj 9643
dg_cast 'teleport' present
%load% obj 9643
dg_cast 'teleport' present
%load% obj 9643
dg_cast 'teleport' present
%load% obj 9643
dg_cast 'teleport' present
%load% obj 9643
dg_cast 'teleport' present
~
#7603
Present 2~
1 g 100
~
eval trapnumber %random.9%
if %trapnumber% == 1
  %echoaround% %actor% %actor.name% opens the present.
  %echoaround% %actor% %actor.name% begins to shift in and out of reality.
  %send% %actor% You feel at home with the &07dead&00.
  dg_cast 'wraithform' %actor.name%
  dg_cast 'blur' %actor.name%
  return 0
  %purge% %self%
elseif %trapnumber% == 2
  %echoaround% %actor% %actor.name% opens the present.
  %echoaround% %actor% A small &13faerie&00 blinks in, and out, of the room.
  %send% %actor% A small &13faerie&00 blinks into the room, waves her wand at you, and vanishes.
  dg_cast 'armor' %actor.name%
  dg_cast 'bless' %actor.name%
  return 0
  %purge% %self%
elseif %trapnumber% == 3
  %echoaround% %actor% %actor.name% opens the present.
  %echoaround% %actor% A single &08red&00 snowflake floats down from above.
  %send% %actor% A single &08red&00 snowflake lands on the tip of your nose.
  dg_cast 'haste' %actor.name%
  dg_cast 'flame blade' %actor.name%
  return 0
  %purge% %self%
elseif %trapnumber% == 4
  %echoaround% %actor% %actor.name% opens the present.
  %echoaround% %actor% A jack-in-the-box SPRINGS out of the box!
  %send% %actor% A jack-in-the-box SPRINGS out of the box and laughs!
  %send% %actor% A jack-in-the-box says, 'Merry Christmas %actor.name%!'
  dg_cast 'sanctuary' %actor.name%
  dg_cast 'shield' %actor.name%
  return 0
  %purge% %self%
elseif %trapnumber% == 5
  %echoaround% %actor% %actor.name% opens the present.
  %echoaround% %actor% The sound of bells can be heard jingling high above.
  %send% %actor% You hear a faint "HO HO HO" and the jingling of bells.
  dg_cast 'pulse gain' %actor.name%
  dg_cast 'pulse heal' %actor.name%
  return 0
  %purge% %self%
elseif %trapnumber% == 6
  %echoaround% %actor% %actor.name% opens the present.
  %echoaround% %actor% %actor.name% has received the gift of knowledge and foresight!
  %send% %actor% You feel the sudden urge to learn.
  %send% %actor% You feel the sudden urge to learn even more.
  dg_cast 'fast learning' %actor.name%
  dg_cast 'true sight' %actor.name%
  return 0
  %purge% %self%
elseif %trapnumber% == 7
  %echoaround% %actor% %actor.name% opens the present.
  %echoaround% %actor% %actor.name% is surrounded by a veil of darkness.
  %send% %actor% You feel as if protected by the gods!
  dg_cast 'shield' %actor.name%
  dg_cast 'sanctuary' %actor.name%
  return 0
  %purge% %self%
elseif %trapnumber% == 8
  %echoaround% %actor% %actor.name% opens the present.
  %echoaround% %actor% %actor.name% is enveloped in &10l&14i&10g&14h&10t&00!
  %send% %actor% All darkness and evil seems to melt away.
  dg_cast 'refresh' %actor.name%
  dg_cast 'refresh' %actor.name%
  dg_cast 'revive' %actor.name%
  return 0
  %purge% %self%
elseif %trapnumber% == 9
  %echoaround% %actor% %actor.name% opens the present.
  %echoaround% %actor% %actor.name% slips into the shadows.
  %send% %actor% You feel as one with the shadows.
  dg_cast 'shadow sphere' %actor.name%
  dg_cast 'shadow vision' %actor.name%
  dg_cast 'dance of shadows' %actor.name%
  return 0
  %purge% %self%
else 
  %echo% There doesn't appear to be anything inside the present. You wonder who took your gift.
  return 0 
end
~
#7611
Grinch Agro -->attached m7615~
0 h 100
~
if %actor.is_npc% != 0 || %actor.level% > 50
   halt
else
dg_cast 'pestilence' %actor%
end
~
#7612
Grinch Blink-->attached to m7615~
0 k 20
~
      if %actor.is_npc% != 0 || %actor.level% > 51
      halt
      else
%echo% The Grinch dissipates into a &02green vaporous&00 cloud and disappears!
      mgoto 7611
wait 3 s
      mgoto 7613
      wait 1 s
      %echo% A &02green vaperous&00 cloud drifts into the room.
      dg_cast 'pestilence'
      wait 1 s
      cackle
      end
~
#7613
Grinch Duplicates-->attached m7615~
0 l 25
~
   Wait 2 s
%echo% The Grinch's back bulges out and a green &02ball of fur&00 pops out of it.
%load% mob 7617
%force% furball assist grinch
wait 400
~
#7614
Claus Greet-->attached m7615~
0 g 60
~
wait 5
say Oh Dear, some of Santa's Christmas elves stole all the ingredients for his Christmas cake.
wait 10
say What shall I do, Christmas is just around the corner.
wait 5
sniff
~
#7615
Flee Killer-->attached m7615~
0 e 100
flee~
wait 1 s
mgoto %actor.name%
%echo% the &02Grinch&00 says, '&22You little rat, you can't escape the Grinch!&00'
mkill %actor.name%
end
~
#7620
Peter Cankersore catapult echo--> attached to r40061~
2 b 18
~
    %echo% Far away you hear a loud whooshing noise
  wait 3 s
    %echo% followed by a loud crack like one tree hitting another!
  wait 3 s
    %echo% You look up to see a cloud of beautifully colored eggs
  wait 3 s
    %echo% being launched far up into the air and raining down in the distance.
  wait 3 s
    %echo% You just barely make out a high-pitched voice yelling, 'This will show them!'
   End
~
#7621
Mother Easter Bunny's ask for help trigger--> attached to m1241~
0 c 100
pe~
set rabbit 'rabbit
if %cmd.mudcommand% == pet && 'rabbit /= '%arg% && %arg.strlen% > 0
  %send% %actor% The Mother Easter Rabbit looks at you with sad eyes.
  wait 15
  %send% %actor% The Mother Easter Rabbit sniffs, 'Somebody has stolen the Easter eggs I've been making all year!'
  wait 30
  %send% %actor% The Mother Easter Rabbit says, 'They are trying to ruin Easter! They've hidden all my eggs!'
  wait 3 sec
  sniff
  wait 3 sec
  cry
  wait 1 sec
  sniff
  wait 5 sec
  %send% %actor% The Mother Easter Rabbit asks, 'Who would DO such a thing!?'
  wait 5 sec
  %send% %actor% The Mother Easter Rabbit is &08FURIOUS&00!!
  wait 1 sec
  %send% %actor% The Mother Easter Rabbit pleads, 'Would you please help me retrieve these eggs?'
  wait 1 sec
  raise
end
~
#7622
Mother Easter Bunny's thank you trigger for eggs--> attached to m1241~
0 j 100
~
      if %object.vnum% == 1657
      say Oh, thank you %actor.name%! You don't know how much this means.
       wait 2 s
      %echo% the Mother Easter Rabbit carefully takes the egg and places it in her basket.
       wait 2 s
      say With your help and all the other searchers, we might yet save Easter!
       wait 2 s
      say But we still need to find the rest of the eggs. Please keep looking!
       else
       wait 1 s
      say This is not one of the Easter eggs I worked so hard on. Please keep looking!
      wait 1 s
      drop all
      endif
~
#7623
Mother Easter Bunny's thank you and 1qp reward for golden eggs--> attached to m1241~
0 j 100
~
if %object.vnum% == 43452
  wait 2 s
  %send% %actor% The Mother Easter Rabbit says, 'Oh, thank you %actor.name%! You don't know how much this means.'
  mjunk o43452
  wait 1 s
  %echo% The Mother Mother Easter Rabbit carefully takes the egg and places it in her basket.
  wait 1 s
  %send% %actor% The Mother Easter Rabbit says, 'With your help and all the other searchers, we might yet save Easter!'
  wait 1 s
  %send% %actor% The Mother Easter Rabbit says,  'But we still need to find the rest of the eggs. Please keep looking!'
  wait 1 s
endif
~
#7624
Mother Easter Bunny's "no" reply to request for help--> attached to m1241~
0 d 100
no~
if %actor.vnum% < 0
  wait 2 s
  %send% %actor% The Mother Easter Rabbit snickers, 'You are a very mean person %actor.name%.'
  wait 2 s
  %send% %actor% The Mother Easter Rabbit says, 'I hope all you get for Christmas is a lump of coal!'
endif
~
#7625
Mother Easter Bunny's "yes" reply to request for help--> attached to m1241~
0 d 100
yes~
if %actor.vnum% < 0
  vwait 2 s
  %send% %actor% The Mother Easter Rabbit winks and says, 'I will give you a special reward for bringing me those eggs.'
  wait 2 s
  %send% %actor% The Mother Easter Rabbit says, 'Type "quest offer" to continue.'
  wait 2 s
  %echo% The Mother Easter Rabbit goes back to worriedly watching the colored eggs fall into the distance.
endif
~
#7686
Christmas Elf 4~
2 f 100
~
%load% mob 7607
dg_cast 'teleport' elf
~
#7687
Pan Cake/Torch~
0 n 100
~
%load% obj 1231
%load% obj 7622
hold torch
~
#7688
Frosty Present Drop~
0 g 20
~
if %actor.is_npc% !=0
  halt
else
  say &13Hey kids! Look who it is! %actor.name%!
  %echo% &13Frosty &00the &14Snowman&00 sinks deeply into his thoughts.
  %load% obj 7688
  drop present
end
~
#7689
Frosty Present Trigger~
1 g 100
~
eval trapnumber %random.6%
if %trapnumber% == 1
  %echoaround% %actor% %actor.name% opens the &13C&14h&13r&14i&13s&14t&13m&14a&13s&00 present.
  %echoaround% %actor% &13Frosty &00the &14Snowman&00 offers %actor.name% a tall glass of &14eggnog&00 and a plate of gingerbread cookies.
  %send% %actor% &13Frosty &00the &14Snowman&00 offers up some &14eggnog&00 and gingerbread cookies. As you eat them, you feel like you could take on ELRU!
  dg_cast 'haste' %actor.name%
  dg_cast 'sanctuary' %actor.name%
  dg_cast 'fly' %actor.name%
  return 0
  %purge% %self%
elseif %trapnumber% == 2
  %echoaround% %actor% %actor.name% opens the &13C&14h&13r&14i&13s&14t&13m&14a&13s&00 present.
  %echoaround% %actor% &13Frosty &00the &14Snowman&00 offers %actor.name% his &00&03Corncob Pipe&00!
  %send% %actor% &13Frosty &00the &14Snowman&00 offers you his &00&03Corncob Pipe!&00 As you take a puff, you feel funny!
  dg_cast 'pulse gain' %actor.name%
  dg_cast 'pulse heal' %actor.name%
  return 0
  %purge% %self%
elseif %trapnumber% == 3
  %echoaround% %actor% %actor.name% opens the &13C&14h&13r&14i&13s&14t&13m&14a&13s&00 present.
  %echoaround% %actor% &13Frosty &00the &14Snowman&00 smiles and offers to sing %actor.name% a Christmas carol!
  %send% %actor% &13Frosty &00the &14Snowman&00 starts to sing a Christmas carol to you!
  dg_cast 'ward' %actor.name%
  dg_cast 'shield' %actor.name%
  dg_cast 'blink' %actor.name%
  dg_cast 'awaken' %actor.name%
  return 0
  %purge% %self%
elseif %trapnumber% == 4
  %echoaround% %actor% %actor.name% opens the &13C&14h&13r&14i&13s&14t&13m&14a&13s&00 present.
  %echoaround% %actor% &13Frosty &00the &14Snowman&00 frowns and look displeased with %actor.name%.
  %send% %actor% &13Frosty &00the &14Snowman&00 doesn't look very happy to see you!
  %send% %actor% With his &00&03Corncob Pipe&00 and &07Button Nose&00, &13Frosty &00the &14Snowman&00 releases his &08wrath.&00
  dg_cast 'silence' %actor.name%
  dg_cast 'silence' %actor.name%
  dg_cast 'silence' %actor.name%
  dg_cast 'silence' %actor.name%
  dg_cast 'silence' %actor.name%
  dg_cast 'blind' %actor.name%
  dg_cast 'blind' %actor.name%
  dg_cast 'blind' %actor.name%
  dg_cast 'blind' %actor.name%
  dg_cast 'blind' %actor.name%
  dg_cast 'paralyze' %actor.name%
  dg_cast 'paralyze' %actor.name%
  dg_cast 'paralyze' %actor.name%
  dg_cast 'paralyze' %actor.name%
  dg_cast 'paralyze' %actor.name%
  %send% %actor% &08Off to Temple to sit out your punishment!&00
  %echoaround% %actor% &14A large hand plucks %actor.name% out of the room.&00
  %teleport% %actor% 18102
  return 0
  %purge% %self%
elseif %trapnumber% == 5
  %echoaround% %actor% %actor.name% opens the &13C&14h&13r&14i&13s&14t&13m&14a&13s&00 present.
  %echoaround% %actor% &13Frosty &00the &14Snowman&00 frowns at %actor.name%. 
  %send% %actor% &13Frosty &00the &14Snowman&00 glowers as his eyes turn &08red&00.
  %send% %actor% &13Frosty &00the &14Snowman&00 mutters something incomprehensible. 
  dg_cast 'dishearten' %actor.name%
  dg_cast 'dishearten' %actor.name%
  dg_cast 'dishearten' %actor.name%
  dg_cast 'dishearten' %actor.name%
  dg_cast 'dishearten' %actor.name%
  dg_cast 'energy drain' %actor.name%
  dg_cast 'energy drain' %actor.name%
  dg_cast 'energy drain' %actor.name%
  dg_cast 'energy drain' %actor.name%
  dg_cast 'energy drain' %actor.name%
  dg_cast 'disease' %actor.name%
  dg_cast 'disease' %actor.name%
  dg_cast 'disease' %actor.name%
  dg_cast 'disease' %actor.name%
  dg_cast 'disease' %actor.name%
  %damage% %actor% 150
  %send% %actor% &13Frosty &00the &14Snowman&00 sucker punches you in the face!
  %send% %actor% OUCH! That really did hurt!
  %echoaround% %actor% &13Frosty &00the &14Snowman&00 sucker punches %actor.name% in the face!
  return 0
  %purge% %self%
elseif %trapnumber% == 6
  %echoaround% %actor% %actor.name% opens the &13C&14h&13r&14i&13s&14t&13m&14a&13s&00 present.
  %echoaround% %actor% &13Frosty &00the &14Snowman&00 winks at %actor.name%.
  %send% %actor% &13Frosty &00the &14Snowman&00 winks at you.
  %send% %actor% &13Frosty &00the &14Snowman&00 says, '&13I hope this helps!&00'
  dg_cast 'fast learning' %actor.name%
  dg_cast 'fast learning' %actor.name%
  dg_cast 'true sight' %actor.name%
  dg_cast 'blur' %actor.name%
  dg_cast 'holy armor' %actor.name%
  return 0
  %purge% %self%
else 
  %echo% There doesn't appear to be anything inside the present. You wonder who took your gift.
  return 0 
end
~
#7690
Sugar/Torch~
0 n 100
~
%load% obj 7615
%load% obj 1231
hold tor
~
#7691
Flour/Torch~
0 n 100
~
%load% obj 1231
%load% obj 7612
hold torch
~
#7692
Butter/Torch~
0 n 100
~
%load% obj 1231
%load% obj 7614 
hold torch
~
#7693
Eggs/Torch~
0 n 100
~
%load% obj 1231
%load% obj 7613
hold torch
~
#7694
Grinchling Load~
2 f 100
~
%load% mob 7616
dg_cast 'teleport' grinchling
%load% mob 7616
dg_cast 'teleport' grinchling
~
#7695
Christmas Elf 1~
2 f 100
~
%load% mob 7609
dg_cast 'teleport' elf
~
#7696
Christmas Elf 2~
2 f 100
~
%load% mob 7610
dg_cast 'teleport' elf
~
#7697
Christmas Elf 3~
2 f 100
~
%load% mob 7611
dg_cast 'teleport' elf
~
#7698
Chocolate/Torch~
0 n 100
~
%load% obj 1231
%load% obj 7617
hold torch
~
#7699
Grinchling Load 2~
2 f 100
~
%load% mob 7608
dg_cast 'teleport' grinchling
~
$~
