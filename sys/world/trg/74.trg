#7402
Crazy Ivan~
0 g 100
~
wait 5
say Ivan present treasures for scampers.
wait 20
say nummy yums!
~
#7409
Merden~
0 g 100
0~
wait 2
mecho Merden jumps in fear.
wait 3
say don't scare me like that!
wait 10
say if you be hunting Bacites than you come to right place.
wait 7
say Oh, and don't ferget to grab up them tails, I hear the watermaster will pay a nice reward fer them.
wait 10
say Good Luck!
end
~
#7413
Ragneli speech 1 trigger~
0 g 60
~
wait 5
say I have a task for a Brave Adventurer such as yourself.
wait 19
say would you be interested?
end
~
#7414
Ragneli speech 2 trigger~
0 d 100
yes what maybe~
wait 3
say Good, just type Quest Offer to receive the task.
end
~
#7415
Ragneli speech 3 trigger~
0 d 100
no~
wait 5
say Then farewell to you.
end
~
#7416
Speech trigger for questmasters~
0 g 70
~
wait 5
say Greetings Adventurer, are you interested in a task for which you will be well rewarded?
end
~
#7417
Speech triggers for Shoppie QM's~
0 g 100
~
wait 5
say Greetings Adventurer, are you interested in a task for which you will be well rewarded?
wait 5
say or are you here to see a list of my fine wares?
end
~
#7418
Rhyvis greet trigger~
0 g 70
~
wait 5
say Greetings Adventurer, have you come for a job?
end
~
#7420
Freds class test trigger~
0 g 100
arrive. arrived.~
wait 2
if %actor.class% == Magic User
say Hello Mage %actor.name% , How are you today?
elseif %actor.class% == Cleric
say Hello Cleric %actor.name% , How are you today?
elseif %actor.class% == Warrior
say Hello Warrior %actor.name% , How are you today?
elseif %actor.class% == Thief
say Hello Thief %actor.name% , please keep your hands off my coins!
elseif %actor.class% == Assassin
say Hello Mercenary %actor.name% , I hope you are not stalking me!
elseif %actor.class% == Ranger
say Hello Ranger %actor.name% , how are the forests this day?
elseif %actor.class% == Death knight
say Hello Dark Lord %actor.name% , I hope you have not come for me!
elseif %actor.class% == Shadow Dancer
say Hello Shadow %actor.name% , I hope darkness is plentiful for you today!
elseif %actor.class% == Solamnic Knight
say Hello Sir Knight %actor.name% , I hope you are well today!
else
say Hello Monk %actor.name% , I hope you are well today!
end
~
#7421
Freds race trigger test~
0 g 100
~
wait 5
if %actor.race% == Draconian
say Need a breathmint %actor.name%?
elseif %actor.race% == Troll
say Hey %actor.name%, you big Troll, get off my foot!
elseif %actor.race% == Elf
say Careful %actor.name% , you almost poked my eye out with them pointy ears!
elseif %actor.race% == Dwarf
say Hey %actor.name% , stand up so we can see you! Oh, you are!
elseif %actor.race% == Minotaur
say Hey %actor.name% , have the rest of the cows come home yet?!
elseif %actor.race% == Ogre
say Hello %actor.name% , how are the warts?
elseif %actor.race% == Gnome
say Look out it's %actor.name% , put on the flac jackets!
elseif %actor.race% == Orc
say Hello %actor.name% , cracked any mirrors today?!
elseif %actor.race% == Human
say Oh look, it's %actor.name% , dinner has arrived!
else
say Hiya %actor.name% , get a job yet?!
end
~
#7422
Freds speech trigger~
0 d 100
bitch bastard ass whore slut fag fuck fucker cunt twat nigger gay asshole suck meat~
wait 3
if %actor% && %victim.vnum% != %self.vnum%
     %echo% Fred says, "%actor.name% , you need your ass kicked."
     kill %actor.name%
     end
~
#7423
Freds slap action trigger~
0 e 100
slapped~
wait 3
if %victim% && %victim.vnum% == %self.vnum%
     %echo% Fred says, "%actor.name% , you need your ass kicked."
     kill %actor.name%
     end
~
#7424
Freds random sayings trigger~
0 bg 20
~
wait 6
eval phrase %random.31%
if %phrase% == 1
say Lets play house. You be the door and I'll slam you.
elseif %phrase% == 2
say We all spring from apes, but you didn't spring far enough. 
elseif %phrase% == 3
say You have a good family tree, but the crop is a failure. 
elseif %phrase% == 4
say We know you could not live without us. We'll pay for the funeral.
elseif %phrase% == 5
say In the dictionary under the word, "stupid," it says, "see him." 
elseif %phrase% == 6
say You're the best at all you do, and all you do is make people hate you.
elseif %phrase% == 7
say Hey, act your age -- senile! 
elseif %phrase% == 8
say It's your life -- but I wish you'd let us have it.
elseif %phrase% == 9
say We know that romance brings out the beast in you -- the jackass.
elseif %phrase% == 10
say Is your name Maple Syrup? It should be, you sap.
elseif %phrase% == 11
say Man alive! But I wish you weren't. 
elseif %phrase% == 12
say There was something about you that I liked, but you spent it. 
elseif %phrase% == 13
say Sit down and give your mind a rest.
elseif %phrase% == 14
say The only things you ever make are mistakes and cigarette ashes. 
elseif %phrase% == 15
say We can always tell when you are lying. Your lips move. 
elseif %phrase% == 16
say Ordinarily people live and learn. You just live.
elseif %phrase% == 17
say When you talk, other people get hoarse just listening.
elseif %phrase% == 18
say I thought of you all day today when I was at the zoo.
elseif %phrase% == 19
say If ignorance is bliss, then why aren't you people happy?
elseif %phrase% == 20
say If you can't beat them, arrange to have them beaten
elseif %phrase% == 21
say I took an IQ test here and the results were negative
elseif %phrase% == 22
say i souport publik edekasion.
elseif %phrase% == 23
say 3 kinds of people: those who can count & those who can't.
elseif %phrase% == 24
say Out of my mind. Back in five minutes.
elseif %phrase% == 25
say He who laughs last thinks slowest.
elseif %phrase% == 26
say Reality? That's where the pizza delivery guy comes from.
elseif %phrase% == 27
say Where there's a will, I want to be in it.
elseif %phrase% == 28
say The gene pool here could use a little chlorine.
elseif %phrase% == 29
say Cover me. I'm changing lanes.
elseif %phrase% == 30
say Cat: The other white meat!
else
say Procrastinators Unite!...Tommorow.
end
~
#7425
Freds spit action trigger~
0 e 100
"spits in"~
wait 3
if %victim% && %victim.vnum% == %self.vnum%
     %echo% Fred says, "%actor.name% , you need your ass kicked."
     kill %actor.name%
     end
~
#7450
graveyard trigger test room 36515~
2 g 50
~
wait 6
eval phrase %random.5%
if %phrase% == 1
%echo% &01Ghastly howls and shrieks echo amongst the tumbled tombstones.&00
elseif %phrase% == 2
%echo% &02A ghostly hand brushes against your face.&00
elseif %phrase% == 3
%echo% &08A cackling skull flies straight at you!&00
elseif %phrase% == 4
%echo% &03The sounds of hands digging through the soil can be heard all around you.&00
else
%echo% &07A rotting hand bursts through the soil and clutches at your leg!&00
end
~
#7451
ghostly wail rm 36514~
2 g 100
~
wait 6
%echo% A chill passes over you as you near the defiled chapel.
end
~
#7452
move alter test~
2 c 100
push~
%echoaround% %actor% You here a cracking as %actor.name% pushes on the alter.
%send% %actor% You hear cracking as you push on the alter.
%echo% The alter starts to move reveling a passage down.
wdoor 36524 down room 36525
wait 100
wdoor 36524 down purg
%echo% The alter slides back into place covering the passage.
end
~
#7453
object evil only test~
1 g 100
~
if %actor.is_npc% || %actor.align% >-600
  %send% %actor% The chalice zaps you with a jolt of electricty.
  %echoaround% %actor% The chalice emits a shower of sparks as it zaps %actor.name%.
  return 0
end
~
#7454
chalice give trigger~
1 i 100
~
if %victim.is_npc% || %victim.align% >-600
  %send% %victim% The chalice zaps you with a jolt of electricty.
  %echoaround% %victim% The chalice emits a shower of sparks as it zaps %victim.name%.
  %damge% %actor% 5
  return 0
end
~
#7455
necro only trigger test~
1 g 100
~
if %actor.class% == warrior
%send% %actor% The chalice zaps you with a jolt of electricty.
%echoaround% %actor% The chalice emits a shower of sparks as it zaps %actor.name%.
return 0
elseif %actor.class% == magic user
%send% %actor% The chalice zaps you with a jolt of electricty.
%echoaround% %actor% The chalice emits a shower of sparks as it zaps %actor.name%.
return 0
elseif %actor.class% == cleric
%send% %actor% The chalice zaps you with a jolt of electricty.
%echoaround% %actor% The chalice emits a shower of sparks as it zaps %actor.name%.
return 0
elseif %actor.class% == ranger
%send% %actor% The chalice zaps you with a jolt of electricty.
%echoaround% %actor% The chalice emits a shower of sparks as it zaps %actor.name%.
return 0
elseif %actor.class% == thief
%send% %actor% The chalice zaps you with a jolt of electricty.
%echoaround% %actor% The chalice emits a shower of sparks as it zaps %actor.name%.
return 0
elseif %actor.class% == death knight
%send% %actor% The chalice zaps you with a jolt of electricty.
%echoaround% %actor% The chalice emits a shower of sparks as it zaps %actor.name%.
return 0
elseif %actor.class% == Solamnic Knight
%send% %actor% The chalice zaps you with a jolt of electricty.
%echoaround% %actor% The chalice emits a shower of sparks as it zaps %actor.name%.
return 0
elseif %actor.class% == Shou-Lin
%send% %actor% The chalice zaps you with a jolt of electricty.
%echoaround% %actor% The chalice emits a shower of sparks as it zaps %actor.name%.
return 0
elseif %actor.class% == Assassin
%send% %actor% The chalice zaps you with a jolt of electricty.
%echoaround% %actor% The chalice emits a shower of sparks as it zaps %actor.name%.
return 0
elseif %actor.class% == Shadow Dancer
%send% %actor% The chalice zaps you with a jolt of electricty.
%echoaround% %actor% The chalice emits a shower of sparks as it zaps %actor.name%.
return 0
end
~
$~
