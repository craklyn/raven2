#1340
Munchkin Treasure (1)~
0 f 100
~
%load% obj 1399
%echo% &14As the munchkin's life fades, forever, some treasure spills out of a pouch they were carrying.&00
~
#1350
Level 1 Helpers~
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
    %load% mob 1370
    eval guardmob %roomid.people%
    %echo% A &09potted plant&00 combs his leaves. What a dashing specimen!
    wait 1 sec
    %echo% A low level munchkin grumbles loudly.
    %force% %guardmob% assist munchkin
    %force% %guardmob% rescue munchkin
    wait 5000
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
    %load% mob 1371
    eval guardmob %roomid.people%
    %echo% Oh no! &00&01CRABS!!&00
    wait 1 sec
    %echo% A low level munchkin sighs loudly.
    %force% %guardmob% assist munchkin
    %force% %guardmob% rescue munchkin
    wait 5000
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
    %load% mob 1372
    eval guardmob %roomid.people%
    %echo% A large &08angry &14chicken&00 pokes his head out from behind a low level munchkin.
    wait 1 sec
    %echo% A low level munchkin says, 'ffs...'
    %force% %guardmob% assist munchkin
    %force% %guardmob% rescue munchkin
    wait 5000
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
    %load% mob 1373
    eval guardmob %roomid.people%
    %echo% A group of &09frogs&00 descend(?) upon you!
    wait 1 sec
    %echo% A low level munchkin pushes a rock around with his big toe.
    %force% %guardmob% assist munchkin
    %force% %guardmob% rescue munchkin
    wait 5000
  break
  case 5
    eval roomid %self.room%
    eval roomname %roomid.name%
    shout &00&02I need a TROLL!&00
    wait 1 sec
    %echo% A low level munchkin rolls a large die. It lands on 2!
    wait 2 sec
    %echo% A low level munchkin licks their mouth and smiles.
    wait 1 sec
    %load% mob 1380
    eval guardmob %roomid.people%
    %echo% A &00&03troll&00 has stopped by to troll your posts and comments!
    wait 1 sec
    %echo% A low level munchkin giggles and points at you.
    %force% %guardmob% assist munchkin
    %force% %guardmob% rescue munchkin
    wait 5000
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
    %load% mob 1370
    eval guardmob %roomid.people%
    %echo% A &09potted plant&00 drops in from the sky.
    wait 1 sec
    %echo% A low level munchkin smirks.
    %force% %guardmob% assist munchkin
    %force% %guardmob% rescue munchkin
    wait 5000
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
    %load% mob 1371
    eval guardmob %roomid.people%
    %echo% A group of tiny &00&01crabs&00 crawls out from behind the low level munchkin.
    wait 1 sec
    %echo% A low level munchkin smirks.
    %force% %guardmob% assist munchkin
    %force% %guardmob% rescue munchkin
    wait 5000
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
    %load% mob 1372
    eval guardmob %roomid.people%
    %echo% A large &08angry &14chicken&00 flops on the scene, albeit late.
    wait 1 sec
    %echo% A low level munchkin sobs loudly.
    %force% %guardmob% assist munchkin
    %force% %guardmob% rescue munchkin
    wait 5000
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
    %load% mob 1373
    eval guardmob %roomid.people%
    %echo% A group of &09frogs&00 fly(?) into the room!
    wait 1 sec
    %echo% A low level munchkin snorts derisively.
    %force% %guardmob% assist munchkin
    %force% %guardmob% rescue munchkin
    wait 5000
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
    %load% mob 1380
    eval guardmob %roomid.people%
    %echo% Oh no! An &00&03internet troll&00 has come to aid the munchkin!
    wait 1 sec
    %echo% A low level munchkin taunts you mercilessly!
    %force% %guardmob% assist munchkin
    %force% %guardmob% rescue munchkin
    wait 5000
  break
  default
    %echo% &10There is a problem with this trigger, please report it to Ish!&00
  break
done
detach 1350 %self.id%
~
$~
