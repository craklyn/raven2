#31501
Cave bear death load helmet~
0 f 100
~
%echo% &08The mighty grizzly is dead!&00
%echo% &08Cackling in triumph, %actor.name% leaps atop the corpse and, with a single blow, lops off its head.&00
mload object 31599
end
~
#31502
wolf death load pelt~
0 f 100
~
%echo% &08With one last howl, the wolf perishes.&00
%echo% &08To your delight, the pelt appears to have for the most part remained whole.&00
mload object 31598
end
~
#31503
move boulder open door~
2 c 100
push~
if %actor.str% < 18
%send% %actor.name% You heave at the boulder, but cannot keep it open.
%echoaround% %actor.name% %actor.name% heaves at the boulder, but cannot keep it open.
else
%send% %actor.name% The boulder rumbles as you heave it aside.
%echoaround% %actor.name% The boulder rumbles as %actor.name% heaves it aside.
wdoor 31516 north room 31520
wdoor 31520 south room 31516
endif
~
#31504
gollum greeting~
0 g 100
~
wait 2 s
bow
wait 2 s
say Welcome, %actor.name%, to the kingdom of Tinisbwana!
wait 2 s
%echo% Golem P'chaua folds his arms across his impossibly broad chest.
wait 1 s
say You will be welcome in the city, but I advise caution whilst venturing through the jungle.
wait 2 s
say The natives are hostile, and many of the plants are poisonous.
wait 2 s
say Although you must already know that, having already trudged so far in this sweltering heat.
wait 2 s
%echo% Golem P'chaua wipes his brow.
end
~
#31505
ask about tinisbwana~
0 d 100
tinisbwana~
wait 2 s
say Aye, through the gate, you can reach my homeland!
wait 1 s
smile
wait 2 s
say An ancient place, indeed, seperated from us by time and space.
wait 2 s
say But though it existed milennia ago, it also exists now!
wait 2 s
say Confusing, is it not?
end
~
#31506
ask about key~
0 d 100
key locked gate chain lock~
wait 2 s
say Locked, eh?
wait 2 s
nod
wait 2 s
say My masters were wise to do so, to protect their homeland.
wait 2 s
say I once had the key, and was entrusted with its guardianship.
wait 2 s
say But the wyrm of Dragg's Peak came and stole it!
wait 2 s
mutter
wait 2 s
say Foul dragon, what I wouldn't give to be able to fly...
wait 2 s
say He lurks in the Hellspires, south of here, to the east of the old guard post.
end
~
#31507
give key for a fly~
0 e 0
stares at you and utters the words, 'yrl'.~
wait 2 s
say By the gods and goddesses! I can fly!
wait 2 s
say Wait here, %actor.name%, and I shall fetch ye the key!
wait 2 s
%echo% Golem P'chaua dashes off, flying high to the southeast, towards Dragg's
peak.
wait 1 s
%echo% He returns a minute later, a bloodied wyrm carcass in one hand.
wait 1 s
%echo% In the other he carries a key, made of ice.
wait 1 s
say A token of my appreciation for this blessing, the door way is now open to
you.
wait 1 s
bow
wait 1 s
say Now I am off to enjoy my new freedom!
wiat 1 s
%echo% Golem P'chaua swoops off into the sky.
mgoto 36549
wait 100 s
mgoto 31519
end
~
#31509
transfer to room 31559~
2 g 100
~
wait 1 s
%send% %actor.name% You suddenly feel yourself falling.
wait 1 s
%teleport% %actor.name% 31559
end
~
#31510
Draggs greeting~
0 g 100
~
wait 2 s
smile
wait 1 s
%echo% Dragg's stands and brushes wood chips off his lap.
wait 2 s
say Well met, fellow adventurer. What brings ye to the Hellspires?
wait 2 s
%echo% Seeing your look of puzzlement at his carefree greeting.
wait 1 s
say Oh, you are probably wondering why I rest here, free of care?
wait 1 s
nod
wait 1 s
say Well, it's like this- I am dead. Yes, yes I know, its almost
wait 1 s
say beyond belief that the mightiest dwarven hero ever would be dead.
wait 2 s
say But there you have it. Best I can tell, some supernatural force is keeping me alive for their own purposes.
wait 2 s
say But I refused to be the object of their humor, and have set up quite a home here.
wait 2 s
smile
wait 2 s
say Feel free to rest as long as you wish, friend.
end
~
#31511
ask about X~
0 d 100
Xchantharus Xchan xchan~
wait 2 s
say Xchantharus? You know of it?
wait 1 s
nod
wait 2 s
say I sought it too, once, long ago...
wait 2 s
sigh
wait 1 s
say It does exist you know, beyond the pillars.
wait 2 s
say Ignore the icy plateau, it is a maze of traps and evil monsters.
wait 2 s
say Head instead through the golem's gates, providing you have the key.
wait 2 s
say I never managed to find it, unfortunately.
end
~
#31512
dragon greeting~
0 g 100
~
wait 1 s
say Foolish whelp! You are idiotic to disturb me in my lair!
wait 1 s
say You want the key, I'll wager... But you can't have it!
wait 1 s
say Unless, of course, you are willing to pay for it...
wait 1 s
say with your LIFE!
dg_cast 'wall of fire'
end
~
#31513
open door in 31519~
2 d 100
appreciation~
if %actor.vnum% == 31593
wdoor 31519 north room 31557
wait 109 s
wdoor 31519 north purg
end
~
#31514
give key to gollum~
0 j 100
~
if %object.vnum% == 31589
wait 2 s
say By the gods and goddesses!
wait 2 s
say You have done me a great favor by returning this key to me, %actor.name%
wait 1 s
%echo% Golem P'chaua unlock the huge lock and opens the door.
wait 1 s
say As token of my appreciation for this blessing, the door way is now open to you.
wait 1 s
bow
wait 1 s
mjunk all
end
~
#31550
credadgix shoot~
0 h 100
~
   if %actor.is_npc% != 0 || %actor.level% > 50
halt
else
shoot bone %actor.name%
end
~
$~
