#29901
Treasure (1)~
0 f 100
~
%load% obj 29999
%echo% &14As the munchkin's life fades, forever, some treasure spills out of a pouch they were carrying.&00
~
#29902
Treasure (2)~
0 f 100
~
%load% obj 29999
%load% obj 29999
%load% obj 29999
%echo% &14As the munchkin's life fades, forever, some treasure spills out of a pouch they were carrying.&00
~
#29903
Treasure (3)~
0 f 100
~
%load% obj 29999
%load% obj 29999
%load% obj 29999
%load% obj 29999
%load% obj 29999
%echo% &14As the munchkin's life fades, forever, some treasure spills out of a pouch they were carrying.&00
~
#29911
Helper Mobs, 50-5 (Level 1)~
0 l 50
~
switch %random.10%
  case 1
    eval roomid %self.room%
    eval roomname %roomid.name%
    shout Fools! Now fear my summoning powers!
    wait 1 sec
    %echo% A low level munchkin lets out a &08bloodcurdling&00 scream!
    wait 2 sec
    %echo% A low level munchkin utters the words, '&14tsar xe laude&00'
    wait 1 sec
    %load% mob 29980
    eval guardmob %roomid.people%
    %echo% A &09potted plant&00 materializes before you.
    wait 1 sec
    %echo% A low level munchkin groans loudly.
    %force% %guardmob% assist munchkin
    %force% %guardmob% rescue munchkin
    wait 400
  break
  case 2
    eval roomid %self.room%
    eval roomname %roomid.name%
    shout Now I shall call for assistance!
    wait 1 sec
    %echo% A low level munchkin lets out a &13chilling&00 howl!
    wait 2 sec
    %echo% A low level munchkin screams the words, '&08mith az'zir tor&00'
    wait 1 sec
    %load% mob 29981
    eval guardmob %roomid.people%
    %echo% A group of tiny &00&01crabs&00 crawls out from behind the low level munchkin.
    wait 1 sec
    %echo% A low level munchkin smirks and says, 'Oh well'
    %force% %guardmob% assist munchkin
    %force% %guardmob% rescue munchkin
    wait 400
  break
  case 3
    eval roomid %self.room%
    eval roomname %roomid.name%
    shout My loyal steed! Come to me in my time of need!
    wait 1 sec
    %echo% A low level munchkin lets out a loud whistle that carries on the &11wind&00.
    wait 2 sec
    %echo% A low level munchkin whispers the words, '&05sji ni'k tan&00'
    wait 1 sec
    %load% mob 29982
    eval guardmob %roomid.people%
    %echo% A large &08angry &14chicken&00 charges into the room!
    wait 1 sec
    %echo% A low level munchkin shakes their head in disbelief.
    %force% %guardmob% assist munchkin
    %force% %guardmob% rescue munchkin
    wait 400
  break
  case 4
    eval roomid %self.room%
    eval roomname %roomid.name%
    shout &08Now you'll be in a bit of trouble!&00
    wait 1 sec
    %echo% A low level munchkin rolls a large die. It lands on 5!
    wait 2 sec
    %echo% A low level munchkin grins.
    wait 1 sec
    %load% mob 29983
    eval guardmob %roomid.people%
    %echo% A group of &09flying frogs&00 descend upon you!
    wait 1 sec
    %echo% A low level munchkin looks unimpressed.
    %force% %guardmob% assist munchkin
    %force% %guardmob% rescue munchkin
    wait 400
  break
  case 5
    eval roomid %self.room%
    eval roomname %roomid.name%
    shout &00&01Come forth! Your master is in need!&00
    wait 1 sec
    %echo% A low level munchkin rolls a large die. It lands on 4!
    wait 2 sec
    %echo% A low level munchkin licks their mouth and smiles.
    wait 1 sec
    %load% mob 29984
    eval guardmob %roomid.people%
    %echo% Someone's hand has just crawled into the room!
    wait 1 sec
    %echo% A low level munchkin sighs loudly.
    %force% %guardmob% assist munchkin
    %force% %guardmob% rescue munchkin
    wait 400
  break
  case 6
    eval roomid %self.room%
    eval roomname %roomid.name%
    shout Now I shall summon forth a demon!!
    wait 1 sec
    %echo% A low level munchkin lets out a &08bloodcurdling&00 scream!
    wait 2 sec
    %echo% A low level munchkin utters the words, '&14tsar xe laude&00'
    wait 1 sec
    %load% mob 29980
    eval guardmob %roomid.people%
    %echo% A &09potted plant&00 materializes before you.
    wait 1 sec
    %echo% A low level munchkin groans loudly.
    %force% %guardmob% assist munchkin
    %force% %guardmob% rescue munchkin
    wait 400
  break
  case 7
    eval roomid %self.room%
    eval roomname %roomid.name%
    shout You think this is all I got?!!?
    wait 1 sec
    %echo% A low level munchkin lets out a &13chilling&00 howl!
    wait 2 sec
    %echo% A low level munchkin screams the words, '&08mith az'zir tor&00'
    wait 1 sec
    %load% mob 29981
    eval guardmob %roomid.people%
    %echo% A group of tiny &00&01crabs&00 crawls out from behind the low level munchkin.
    wait 1 sec
    %echo% A low level munchkin smirks and says, 'Oh well'
    %force% %guardmob% assist munchkin
    %force% %guardmob% rescue munchkin
    wait 400
  break
  case 8
    eval roomid %self.room%
    eval roomname %roomid.name%
    shout Now I shall attempt to summon my mighty steed!
    wait 1 sec
    %echo% A low level munchkin lets out a loud whistle that carries on the &11wind&00.
    wait 2 sec
    %echo% A low level munchkin whispers the words, '&05sji ni'k tan&00'
    wait 1 sec
    %load% mob 29982
    eval guardmob %roomid.people%
    %echo% A large &08angry &14chicken&00 charges into the room!
    wait 1 sec
    %echo% A low level munchkin shakes their head in disbelief.
    %force% %guardmob% assist munchkin
    %force% %guardmob% rescue munchkin
    wait 400
  break
  case 9
    eval roomid %self.room%
    eval roomname %roomid.name%
    shout &08Ha. Here comes something wicked!!&00
    wait 1 sec
    %echo% A low level munchkin rolls a large die. It lands on 3!
    wait 2 sec
    %echo% A low level munchkin laughs out loud.
    wait 1 sec
    %load% mob 29983
    eval guardmob %roomid.people%
    %echo% A group of flying &09amphibians&00 have flown in, full speed!
    wait 1 sec
    %echo% A low level munchkin sniffs sadly.
    %force% %guardmob% assist munchkin
    %force% %guardmob% rescue munchkin
    wait 400
  break
  case 10
    eval roomid %self.room%
    eval roomname %roomid.name%
    shout &14Now you're in for it!&00
    wait 1 sec
    %echo% A low level munchkin rolls a large die. It lands on 2!
    wait 2 sec
    %echo% A low level munchkin licks their mouth and smiles.
    wait 1 sec
    %load% mob 29990
    eval guardmob %roomid.people%
    %echo% Oh no! An &00&03internet troll&00 has come to aid the munchkin!
    wait 1 sec
    %echo% A low level munchkin taunts you mercilessly!
    %force% %guardmob% assist munchkin
    %force% %guardmob% rescue munchkin
    wait 400
  break
  default
    %echo% &10There is a problem with this trigger, please report it to Ish!&00
  break
done
detach mob munchkin 29911
~
#29912
Helper Mobs, 50-5 (Level 2)~
0 l 50
~
switch %random.10%
  case 1
    eval roomid %self.room%
    eval roomname %roomid.name%
    shout Heeheehee! Behold!
    wait 1 sec
    %echo% A low level munchkin lowers his head and begins to chant.
    wait 2 sec
    %echo% A low level munchkin chants, '&10siw ij nah, baka&00'
    wait 1 sec
    %load% mob 29980
    eval guardmob %roomid.people%
    %echo% A &09potted plant&00 growls menacingly!
    wait 1 sec
    %echo% A low level munchkin throws his hands up in the air.
    %force% %guardmob% assist munchkin
    %force% %guardmob% rescue munchkin
    wait 400
  break
  case 2
    eval roomid %self.room%
    eval roomname %roomid.name%
    shout My brethren! Your brother is under siege!
    wait 1 sec
    %echo% A low level munchkin throws some &14dust&00 into the air.
    wait 2 sec
    %echo% A low level munchkin snaps his fingers and says, '&07mih daran ti&00'
    wait 1 sec
    %load% mob 29984
    eval guardmob %roomid.people%
    %echo% An undead appendage flops from above you!
    wait 1 sec
    %echo% A low level munchkin giggles with glee.
    %force% %guardmob% assist munchkin
    %force% %guardmob% rescue munchkin
    wait 400
  break
  case 3
    eval roomid %self.room%
    eval roomname %roomid.name%
    shout You fools! When will you LEARN!?
    wait 1 sec
    %echo% A low level munchkin lets out a loud whistle.
    wait 2 sec
    %echo% A low level munchkin whispers the words, '&05dri nok suneh&00'
    wait 1 sec
    %load% mob 29982
    eval guardmob %roomid.people%
    %echo% A large &08angry &14chicken&00 pokes his head out from behind a low level munchkin.
    wait 1 sec
    %echo% A low level munchkin growls.
    %force% %guardmob% assist munchkin
    %force% %guardmob% rescue munchkin
    wait 400
  break
  case 4
    eval roomid %self.room%
    eval roomname %roomid.name%
    shout &08Time to roll a die!&00
    wait 1 sec
    %echo% A low level munchkin rolls a large die. It lands on 1!
    wait 2 sec
    %echo% A low level munchkin moans loudly.
    wait 1 sec
    %load% mob 29983
    eval guardmob %roomid.people%
    %echo% A group of &09flying frogs&00 flutter on in.
    wait 1 sec
    %echo% A low level munchkin pushes a rock around with his big toe.
    %force% %guardmob% assist munchkin
    %force% %guardmob% rescue munchkin
    wait 400
  break
  case 5
    eval roomid %self.room%
    eval roomname %roomid.name%
    shout &00&01Come forth! Your master is in need!&00
    wait 1 sec
    %echo% A low level munchkin rolls a large die. It lands on 4!
    wait 2 sec
    %echo% A low level munchkin licks their mouth and smiles.
    wait 1 sec
    %load% mob 29990
    eval guardmob %roomid.people%
    %echo% Oh no. A &00&03troll&00 has stopped by to troll your posts!
    wait 1 sec
    %echo% A low level munchkin giggles andpoints at you.
    %force% %guardmob% assist munchkin
    %force% %guardmob% rescue munchkin
    wait 400
  break
  case 6
    eval roomid %self.room%
    eval roomname %roomid.name%
    shout Now I shall summon forth a demon!!
    wait 1 sec
    %echo% A low level munchkin recites a &07black&00 scroll!
    wait 2 sec
    %echo% A low level munchkin utters the words, '&14bit d'or san&00'
    wait 1 sec
    %load% mob 29980
    eval guardmob %roomid.people%
    %echo% A &09potted plant&00 falls from the sky.
    wait 1 sec
    %echo% A low level munchkin groans loudly.
    %force% %guardmob% assist munchkin
    %force% %guardmob% rescue munchkin
    wait 400
  break
  case 7
    eval roomid %self.room%
    eval roomname %roomid.name%
    shout You wish to feel PAIN!?
    wait 1 sec
    %echo% A low level munchkin lets out a heavy grunt.
    wait 2 sec
    %echo% A low level munchkin screams the words, '&08zan es prau!&00'
    wait 1 sec
    %load% mob 29981
    eval guardmob %roomid.people%
    %echo% A group of tiny &00&01crabs&00 crawls out from behind the low level munchkin.
    wait 1 sec
    %echo% A low level munchkin smirks.
    %force% %guardmob% assist munchkin
    %force% %guardmob% rescue munchkin
    wait 400
  break
  case 8
    eval roomid %self.room%
    eval roomname %roomid.name%
    shout Terror of the sky! Assist me!
    wait 1 sec
    %echo% A low level munchkin lets out a loud whistle that carries on the &11wind&00.
    wait 2 sec
    %echo% A low level munchkin whispers the words, '&05sji ni'k tan&00'
    wait 1 sec
    %load% mob 29982
    eval guardmob %roomid.people%
    %echo% A large &08angry &14chicken&00 charges into the room!
    wait 1 sec
    %echo% A low level munchkin shakes their head in disbelief.
    %force% %guardmob% assist munchkin
    %force% %guardmob% rescue munchkin
    wait 400
  break
  case 9
    eval roomid %self.room%
    eval roomname %roomid.name%
    shout &08Ha! Time to die now!&00
    wait 1 sec
    %echo% A low level munchkin rolls a large die. It lands on 5!
    wait 2 sec
    %echo% A low level munchkin laughs out loud.
    wait 1 sec
    %load% mob 29983
    eval guardmob %roomid.people%
    %echo% A group of flying &09amphibians&00 have flown in, full speed!
    wait 1 sec
    %echo% A low level munchkin sniffs sadly.
    %force% %guardmob% assist munchkin
    %force% %guardmob% rescue munchkin
    wait 400
  break
  case 10
    eval roomid %self.room%
    eval roomname %roomid.name%
    shout &14No! Enough screwing around!&00
    wait 1 sec
    %echo% A low level munchkin rolls a large die. It lands on 2!
    wait 2 sec
    %echo% A low level munchkin licks their mouth and smiles.
    wait 1 sec
    %load% mob 29990
    eval guardmob %roomid.people%
    %echo% Oh no! An &00&03internet troll&00 has come to aid the munchkin!
    wait 1 sec
    %echo% A low level munchkin taunts you mercilessly!
    %force% %guardmob% assist munchkin
    %force% %guardmob% rescue munchkin
    wait 400
  break
  default
    %echo% &10There is a problem with this trigger, please report it to Ish!&00
  break
done
detach mob munchkin 29912
~
#29913
Helper Mobs, 5-50 (Level 3)~
0 l 50
~
switch %random.10%
  case 1
    eval roomid %self.room%
    eval roomname %roomid.name%
    shout Muahahaha!! Fear my wrath, loser!
    wait 1 sec
    %echo% A low level munchkin winks at you suggestively.
    wait 2 sec
    %echo% A low level munchkin chants, '&09int arz trine&00'
    wait 1 sec
    %load% mob 29980
    eval guardmob %roomid.people%
    %echo% A &09potted plant&00 combs his leaves. What a dashing specimen!
    wait 1 sec
    %echo% A low level munchkin grumbles loudly.
    %force% %guardmob% assist munchkin
    %force% %guardmob% rescue munchkin
    wait 400
  break
  case 2
    eval roomid %self.room%
    eval roomname %roomid.name%
    shout You wish that you had something to stop me!
    wait 1 sec
    %echo% A low level munchkin drops a small &14pin&00.
    wait 2 sec
    %echo% A low level munchkin claps his hands and says, '&02qi aht fo'or&00'
    wait 1 sec
    %load% mob 29981
    eval guardmob %roomid.people%
    %echo% Oh no! &00&01CRABS!!&00
    wait 1 sec
    %echo% A low level munchkin sighs loudly.
    %force% %guardmob% assist munchkin
    %force% %guardmob% rescue munchkin
    wait 400
  break
  case 3
    eval roomid %self.room%
    eval roomname %roomid.name%
    shout FOOLS! You should know better!
    wait 1 sec
    %echo% A low level munchkin lets out a loud scream!
    wait 2 sec
    %echo% A low level munchkin screams the words, '&04yrd an xim!&00'
    wait 1 sec
    %load% mob 29982
    eval guardmob %roomid.people%
    %echo% A large &08angry &14chicken&00 pokes his head out from behind a low level munchkin.
    wait 1 sec
    %echo% A low level munchkin says, 'ffs...'
    %force% %guardmob% assist munchkin
    %force% %guardmob% rescue munchkin
    wait 400
  break
  case 4
    eval roomid %self.room%
    eval roomname %roomid.name%
    shout &08Time to roll a die, then time for YOU to die!!&00
    wait 1 sec
    %echo% A low level munchkin rolls a large die. It lands on 5!
    wait 2 sec
    %echo% A low level munchkin giggles and points at you.
    wait 1 sec
    %load% mob 29992
    eval guardmob %roomid.people%
    %echo% A LOT of &00&01Orcs&00 begin to file in behind a low level munchkin.
    wait 1 sec
    %echo% A low level munchkin pushes a rock around with his big toe.
    %force% %guardmob% assist munchkin
    %force% %guardmob% rescue munchkin
    wait 400
  break
  case 5
    eval roomid %self.room%
    eval roomname %roomid.name%
    shout &00&01I need a TROLL!&00
    wait 1 sec
    %echo% A low level munchkin rolls a large die. It lands on 2!
    wait 2 sec
    %echo% A low level munchkin licks their mouth and smiles.
    wait 1 sec
    %load% mob 29990
    eval guardmob %roomid.people%
    %echo% A &00&03troll&00 has stopped by to troll your posts and comments!
    wait 1 sec
    %echo% A low level munchkin giggles and points at you.
    %force% %guardmob% assist munchkin
    %force% %guardmob% rescue munchkin
    wait 400
  break
  case 6
    eval roomid %self.room%
    eval roomname %roomid.name%
    shout Now I shall summon forth a spawn of hell!!
    wait 1 sec
    %echo% A low level munchkin recites a &08red&00 scroll!
    wait 2 sec
    %echo% A low level munchkin utters the words, '&11ti wik s'or&00'
    wait 1 sec
    %load% mob 29980
    eval guardmob %roomid.people%
    %echo% A &09potted plant&00 drops in from the sky.
    wait 1 sec
    %echo% A low level munchkin smirks.
    %force% %guardmob% assist munchkin
    %force% %guardmob% rescue munchkin
    wait 400
  break
  case 7
    eval roomid %self.room%
    eval roomname %roomid.name%
    shout &08You wish to feel PAIN, DO YOU!?&00
    wait 1 sec
    %echo% A low level munchkin crisply snaps his fingers.
    wait 2 sec
    %echo% A low level munchkin screams the words, '&10pix dra z'el nar!&00'
    wait 1 sec
    %load% mob 29981
    eval guardmob %roomid.people%
    %echo% A group of tiny &00&01crabs&00 crawls out from behind the low level munchkin.
    wait 1 sec
    %echo% A low level munchkin smirks.
    %force% %guardmob% assist munchkin
    %force% %guardmob% rescue munchkin
    wait 400
  break
  case 8
    eval roomid %self.room%
    eval roomname %roomid.name%
    shout I shall summon a beast of GREAT POWER!
    wait 1 sec
    %echo% A low level munchkin closes his eyes and the &11wind&00 swirls.
    wait 2 sec
    %echo% A low level munchkin whispers the words, '&05jyd xor geksh'an&00'
    wait 1 sec
    %load% mob 29982
    eval guardmob %roomid.people%
    %echo% A large &08angry &14chicken&00 flops on the scene, albeit late.
    wait 1 sec
    %echo% A low level munchkin sobs loudly.
    %force% %guardmob% assist munchkin
    %force% %guardmob% rescue munchkin
    wait 400
  break
  case 9
    eval roomid %self.room%
    eval roomname %roomid.name%
    shout &08Ha! Time to die!&00
    wait 1 sec
    %echo% A low level munchkin rolls a large die. It lands on 2!
    wait 2 sec
    %echo% A low level munchkin laughs out loud.
    wait 1 sec
    %load% mob 29992
    eval guardmob %roomid.people%
    %echo% 3,892 &00&01Orcs&00 come wandering in.
    wait 1 sec
    %echo% A low level munchkin snorts derisively.
    %force% %guardmob% assist munchkin
    %force% %guardmob% rescue munchkin
    wait 400
  break
  case 10
    eval roomid %self.room%
    eval roomname %roomid.name%
    shout &14No! Enough screwing around!&00
    wait 1 sec
    %echo% A low level munchkin rolls a large die. It lands on 6!
    wait 2 sec
    %echo% A low level munchkin dances a jig.
    wait 1 sec
    %load% mob 29990
    eval guardmob %roomid.people%
    %echo% Oh no! An &00&03internet troll&00 has come to aid the munchkin!
    wait 1 sec
    %echo% A low level munchkin taunts you mercilessly!
    %force% %guardmob% assist munchkin
    %force% %guardmob% rescue munchkin
    wait 400
  break
  default
    %echo% &10There is a problem with this trigger, please report it to Ish!&00
  break
done
detach mob munchkin 29913
~
#29950
Guard Mob Test~
0 l 50
~
eval roomid %self.room%
eval roomname %roomid.name%
shout Fools! Now fear my summoning powers!
wait 1 sec
%echo% A low level munchkin lets out a &08bloodcurdling&00 scream!
wait 2 sec
%echo% A low level munchkin utters the words, '&14tsar xe laude&00'
wait 1 sec
%load% mob 29980
eval guardmob %roomid.people%
%echo% A &09potted plant&00 materializes before you.
wait 1 sec
%echo% A low level munchkin groans loudly.
%force% %guardmob% rescue mun
%force% %guardmob% assist mun
wait 400
~
#29990
Treasure (1)~
0 f 100
~
%load% obj 29899
%echo% &14As the munchkin's life fades, forever, some treasure spills out of a pouch they were carrying.&00
~
#29999
Load some treasure~
0 l 10
~
%load% obj 29999
&14As the munchkin nears death they begin to try and offer some treasure in exchange for their life.&00
~
$~
