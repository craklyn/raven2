#36501
Necro Grab Only --> o36521~
1 g 100
~
if %actor.level% > 50
halt
end
if %actor.class% != necromancer || %actor.vnum% > 0
%send% %actor.name% The chalice zaps you with a bolt of white fire.
%echoaround% %actor.name% The chalice emits a bolt of white fire as it burns %actor.name%.
%damage% %actor.name% 100
return 0
endif
~
#36502
Necro Give Only -->o36521~
1 i 100
~
if %actor.level% > 50
halt
end
if %victim.class% != necromancer || %victim.vnum% < 0
%send% %victim.name% The chalice zaps you with a bolt of white fire.
%echoaround% %victim.name% The chalice emits a bolt of white fire as it burns %victim.name%.
return 0
endif
~
#36503
graveyard scare trigger~
2 g 50
~
if %actor.is_npc% != 0 || %actor.level% > 50
   halt
end
eval phrase %random.5%
wait 1 s
if %phrase% == 1
%echo% &01Ghastly howls and shrieks echo amongst the tumbled tombstones.&00
elseif %phrase% == 2
%echo% &02A ghostly hand brushes against your face.&00
elseif %phrase% == 3
%echo% A cackling &14skull&00 flies straight at you!
elseif %phrase% == 4
%echo% &05The sounds of hands digging through the soil can be heard all around you.&00
elseif %phrase% == 5
%echo% &04A rotting hand bursts through the soil and clutches at your leg!&00
end
~
#36504
ghostly wail trigger~
2 g 100
~
if %actor.is_npc% != 0 || %actor.level% > 50
halt
else
wait 1 s
%echo% A chill passes over you as you near the defiled chapel.
endif
~
#36505
move alter trigger~
2 c 100
push~
%echoaround% %actor% You hear a cracking as %actor.name% pushes on the alter.
%send% %actor% You hear cracking as you push on the alter.
%echo% The alter starts to move reveling a passage down.
wdoor 36524 down room 36525
wdoor 36525 up room 36524
wait 10 s
wdoor 36524 down purg
wdoor 36525 up purg
%echo% The alter slides back into place covering the passage.
end
~
#36506
knight manor speech trigger~
0 g 100
~
if %direction% == south
wait 1 s
%echo% A Knight of the Manor snaps to attention as you approch.
wait 1 s
say Halt! Stranger, be aware that you have entered, knowingly or otherwise, the domain of our Lord of the Manor, Baron duChesne.
wait 1 s
say The Baron has little patience for rowdy or unruly adventures, or for adventurers at all, so be on your best behavoir!
wait 1 s
say Or face the wrath of his knights!
wait 1 s
say we have enough troubles with the unholy goings on around here.
wait 2 s
%echoaround% %actor% The Knight Manor glares at %actor.name%
%send% %actor% A Knight of the Manor glares at you.
endif
~
#36507
manor knight trouble question~
0 d 100
trouble help problems~
wait 1 s
say Trouble?
wait 1 s
%echo% &06Although the knight attempts to scoff, his nervous manner betrays his fear.&00
wait 1 s
frown
wait 1 s
say Aye, 'tis true, this place is cursed, by that vile necromancer!
wait 1 s
sigh
wait 1 s
say If only I were twenty years younger...
end
~
#36508
witch and necro speech for Knight trigger~
0 d 100
witch witch? necro necromancer~
wait 1 s
%echo% The Knight frowns and lowers his voice nervously.
wait 1 s
say I know naught of these scoundrels, stranger...
wait 1 s
say 'Tis best you speak to the Baron of such matters.
wait 1 s
%echo% &03Although the knight's bearing is fair, you suspect he is hiding something.&00
wait 1 s
%echo% The Knight glances around quickly and steps back to his post.
end
~
#36509
horse greet trigger~
0 g 100
~
if %direction% == south
wait 5 s
%echo% The horse whinnies in anticipation of a fray.
end
~
#36510
talking marauder trigger~
0 g 100
~
if %actor.is_npc% != 0 || %actor.level% > 50
halt
else
wait 1 s
say Living thingggsssss...Feast, my brothers, feast!
wait 1 s
%echo% The ghoul cackles frightfully!
wait 1 s
kill %actor.name%
end
~
#36511
skeleton from grave trigger~
2 g 100
~
if %actor.is_npc% != 0 || %actor.level% > 50
halt
else
wait 1 s
%echo% &06Bones clatter as a skeleton struggles out of a grave.&00
%load% mob 36501
%force% %actor.name% look skele
endif
~
#36512
skeleton aggro trigger~
0 e 100
looks~
say I have a bone to pick with you %actor.name%!
mkill %actor.name%
end
~
#36513
rotting corpse load trigger~
2 g 100
~
if %actor.is_npc%
halt
else
Wait 1 s
%echo% A rotting corpse pulls himself up using a tombstone.
%load% mob 36500
%force% %actor.name% look rott
endif
~
#36514
rotting corpse aggro trigg~
0 e 100
looks~
wait 1 s
flex
wait 1 s
say I smell fresh meat!
wait 1 s
look %actor.name%
wait 2 s
mkill %actor.name%
end
~
#36515
mad priest random trigger~
0 g 100
~
wait 1 s
%echo% The mad priest says 'The Necromancer cometh!'
wait 2 s
%echo% The mad priest moans.
wait 1 s
%echo% The mad priest says 'Ware the ghouls what live beneath us!!!'
wait 2 s
%echo% The mad priest cowers deeper in to the corner.
wait 1 s
%echo% The mad priest says 'G-g-go away...heh...evil here... yes...evil!'
end
~
#36516
mad priest question trigger~
0 d 100
witch necro necromancer~
wait 1 s
%echo% The mad priest screams and covers his ear, stomping his feet on the floor!
wait 3 s
say NO! I WILL HEAR NO MORE!
wait 1 s
%echo% The mad priest goes silent and cowers back in to the corner.
end
~
#36517
mad mage look trigger~
0 e 100
looks~
wait 2 s
%echo% The mad priest goes back to scribbling on the wall.
end
~
#36518
wolf growl trigger~
0 g 100
~
if %actor.is_npc% != 0 || %actor.level% > 50
halt
else
wait 1 s
growl
Wait 1 s
kill %actor.name%
end
~
#36519
on load cackle~
0 n 100
~
wait 1 s
cackle
end
~
#36520
teleport Tarrant to room 36528~
2 g 100
~
%teleport% tarrant 36528
end
~
#36521
Tarrant speech trigger~
0 ag 100
~
wait 3 s
bow
Wait 3 s
say You have violated my tomb, and that is a heinous deed indeed.
wait 2 s
say But you are not the first, and won't be the last,
wait 2 s
say so I will not begrudge you for the sins of thousands.
wait 2 s
sigh
wait 3 s
say I would not bore your ears with my misfortunes.
wait 2 s
say Suffice to say,  I have been condemned to this wretched existence by the vile witch Tus'schepteba.
wait 2 s
say And I may not rest until her damned soul has been consigned to the fiery
pits of hell itself.
wait 2 s
say or my own sad soul is released by the hands of my comrades.
wait 3 s
%echo% Tarrant screams, '&08TUS'SCHEPTEBA!&00'
wait 3 s
%echo% &06Tarrant weeps softly, for his lands and his people.&00
end
~
#36522
Tus question trigger for tarrant~
0 d 100
tus tus' schepteba witch~
wait 1 s
snarl
wait 2 s
say Speak not that unworldy slut's name in my presence!
wait 2 s
say She is vile hellspawned inculcable evil!
wait 2 s
say She needs be destroyed, aye, and soon!
wait 2 s
say But I am entombed here, and she is free to wander!
wait 2
say How it tasks me!
wait 2 s
%echo% &06Tarrant gnashes his teeth and mutters in rage.&00
end
~
#36523
tarrant necro question trigger~
0 d 100
necro necromancer~
wait 2 s
say The necromancer?
wait 2 s
mutter
wait 2 s
say Few indeed know of this dark presence. 'Tis rumored he is the true mastermind behind the workings of the witch. 
wait 2 s
say but 'tis also rumored that the witch and the necromancer are but two sides of the same coin... Who can know?
wait 2 s
say Truely I do not.
end
~
#36524
pull torch open wall trigger~
1 c 4
*~
if (%cmd% == pull || %cmd% == pul)
if (%self.name% /= %arg% && %arg.strlen% > 0)
if (%actor.is_npc% != 0 || %actor.level% > 50)
halt
else
%echoaround% %actor% You hear a click as %actor.name% pulls on the torch.
%send% %actor% You hear click as you pull on the torch.
%echo% The wall starts to move reveling a passage south.
%door% 36525 south room 36526
%door% 36526 north room 36525
wait 50
%door% 36525 south purg
%door% 36526 north purg
%echo% The wall slides back into place covering the passage.
end
else
return 0
end
else
return 0
end
~
#36525
pull torch move wall n trigger~
1 c 4
*~
if (%cmd% == pull || %cmd% == pul)
if (%self.name% /= %arg% && %arg.strlen% > 0)
if (%actor.is_npc% != 0 || %actor.level% > 50)
halt
else
%echoaround% %actor% You hear a click as %actor.name% pulls on the torch.
%send% %actor% You hear click as you pull on the torch.
%echo% The wall starts to move reveling a passage south.
%door% 36525 south room 36526
%door% 36526 north room 36525
wait 50
%door% 36525 south purg
%door% 36526 north purg
%echo% The wall slides back into place covering the passage.
end
else
return 0
end
else
return 0
end
~
#36526
knight only orb get trigger~
1 g 100
~
if %actor.level% > 50
halt
end
if %actor.class% != Solamnic Knight || %actor.vnum% > 0
%send% %actor.name% The orb zaps you with a bolt of white fire.
%echoaround% %actor.name% The orb emits a bolt of white fire as it burns %actor.name%.
%damage% %actor.name% 100
return 0
endif
~
#36527
orb knight only give trigger~
1 i 100
~
if %victim.vnum% < 0
if %victim.class% != Solamnic Knight
%send% %victim.name% The orb zaps you with a bolt of white fire.
%echoaround% %vicim.name% The orb emits a bolt of white fire as it burns %victim.name%.
return 0
endif
endif
~
#36528
Load orb on Tarrant freedom word~
2 d 100
freedom~
if %actor.vnum% > 0
wait 5 s
%echo% A bright white light seems to settle over the coffin.
wload obj 36524
endif
~
#36529
load parchment run2~
2 ac 100
move~
if %alreadyrun2% == Yes
halt
end
%echoaround% %actor% You hear something fall as %actor.name% moves the board.
%send% %actor% You hear something fall as you move the board.
wait 1 s
%echo% A piece of parchment falls from behind the board.
%load% obj 36525
eval alreadyrun2 Yes
global alreadyrun2
~
#36530
tarrant hitpoints mob load~
0 l 90
~
%echo% &08Sorely pressed, Tarrant calls upon the favor of the fallen!&00
wait 1 s
%echo% &08Lord Tarrant utters the words, 'daed eht fo sthgink'.&00
wait 2 s
%echo% &08A ghostly knight steps out of one of the portraits.&00
%load% mob 36513
wait 1 s
%force % portrait bow
wait 1 s
%force% portrait say I exist to serve thee, my lord.
wait 1 s
%force% portrait assist tarrant
wait 10 s
end
~
#36531
load orb on tarrant death~
0 l 1
~
say Freedom at last!
wait 100 s
end
~
#36532
teleport G'gor to 36536~
2 ag 100
~
if %alreadyrun3% == Yes
halt
end
%teleport% g'qor 36536
eval alreadyrun3 Yes
global alreadyrun3
end
~
#36533
G'gor greet speech~
0 g 100
~
if %actor.is_npc% != 0 || %actor.level% > 50
halt
else
wait 1 s
%echo% The shaman looks up as you enter.
wait 1 s
say Ah, %actor.name% Well met indeed, I have been expecting you.
wait 1 s
%echoaround% %actor.name% &06Noting %actor.name% surprise, he nods and continues.&00
%send% %actor.name% &06Noting your surprise, G'gor nods and continues.&00
wait 1 s
say Yes, the Cabal has ears everywhere, %actor.name%. Your mission is known.
wait 1 s
chuckle
wait 1 s
say Rather amusing really. What can you possibly hope to achieve?
wait 1 s
shrug
wait 1 s
say Yes, we are aware you seek them. I suppose you desire to rid the world of their evil...
wait 1 s
say ...or take over for yourself, blackheart...
wait 1 s
poke %actor.name%
wait 1 s
snicker
wait 1 s
nod
wait 1 s
say Enough talk. Time for action.
wait 1 s
%echo% &06The lieutenant rises to his full ten feet and draws a large flamberge.&00
wait 1 s
say Time to die, %action.name% May whatever god you worship take pity on your soul.
wait 1 s
kill %actor.name%
end
~
#36534
G'gor call for helpers~
0 l 20
~
%echo% &06Sensing defeat is imminent, G'qor calls upon dark powers to aid him!&00
wait 2 s
%echo% &06G'qor utters the words, 'em dia ssenkrad'&00
%load% mob 36511
wait 1 s
%echo% &08A demonic creature appears in a flash of hellish light.&00
wait 1 s
%force% demonic assist lieutenant
end
~
#36535
Damn priest greet~
0 g 100
~
wait 2 s
drool
wait 2 s
%echo% The priest's eyes roll in their sockets as he turns to face you.
if %actor.sex% == female
wait 2 s
say Sister %actor.name% Come, come unto the purity of my master!
wait 2 s
%send% %actor.name% The priest beckons to you, cackling and frothing at the mouth.
%echoaround% %actor.name% The priest beckons to %actor.name%, cackling and frothing at the mouth.
wait 2 s
%echo% &08The priest draws a long knife across his arm, drawing blood!&00
wait 2 s
%echo% &08The priest shudders in unholy ecstacy.&00
wait 2 s
say You seek death so willingly?
wait 2 s
say Then face it, %actor.name%, face it!
wait 2 s
%send% %actor.name% The priest suddenly lunges at you, sending you tumbling backwards.
%echoaround% %actor.name% The priest suddenly lunges at %actor.name%, sending them tumbling backwards.
%force% %actor.name% down
wait 1 s
%echo% The Priest jumps in to the pentagram and is swallowed up.
mgoto 36550
else
wait 2 s
say Brother %actor.name% Come, come unto the purity of my master!
wait 2 s
%send% %actor.name% The priest beckons to you, cackling and frothing at the mouth.
%echoaround% %actor.name% The priest beckons to %actor.name%, cackling and frothing at the mouth.
wait 2 s
%echo% &08The priest draws a long knife across his arm, drawing blood!&00
wait 2 s
%echo% &08The priest shudders in unholy ecstacy.&00
wait 2 s
say You seek death so willingly?
wait 2 s
say Then face it, %actor.name%, face it!
wait 2 s
%send% %actor.name% The priest suddenly lunges at you, sending you tumbling backwards.
%echoaround% %actor.name% The priest suddenly lunges at %actor.name%, sending them tumbling backwards.
%force% %actor.name% down
wait 1 s
%echo% The Priest jumps in to the pentagram and is swallowed up.
mgoto 36550
endif
~
#36536
mortus beast greeting~
0 g 100
~
wait 1 s
kill %actor.name%
end
~
#36537
Tus onload trigger~
0 n 100
~
wait 2 s
say So, you have managed to find my little lair...
wait 2 s
chuckle
wait 2 s
%echo% Tus shrugs and waves a clawed hand casually.
wait 2 s
say It matters not. Soon the Necromancer will come,
wait 1 s
say and then it will be far too late...
wait 2 s
say Now if you'll excuse me, I have a meeting to attend...
wait 2 s
cackle
wait 1 s
%echo% The witch disappears in a cloud of dark energy!
mgoto 36549
end
~
#36538
load tus clone run1~
2 ag 100
~
if %alreadyrun1% == Yes
halt
end
wait 2 s
%echo% As you enter, a hideous crone shimmers into view.
%load% mob 36505
eval alreadyrun1 Yes
global alreadyrun1
~
#36539
Francois greet trigger~
0 g 100
~
wait 1 s
smile %actor.name%
wait 1 s
say Ho! If it isn't %actor.name%, the renowned adventurer!
wait 2 s
applaud
Wait 1 s
say I know of thee. Welcome, welcome to Gilchrist!
wait 2 s
%echo% &06Although the Burgomeister appears jovial, it seems somehow forced.&00
wait 1 s
%echo% &06What could be troubling this merry fellow?&00
end
~
#36540
Francois trouble question~
0 d 100
trouble necro necromancer~
wait 2 s
say Funny you should ask, %actor.name%.
wait 1 s
%echo% &07The moon suddenly turns dark.&00
wait 2 s
say The minions of the Necromancer are about... 
wait 1 s
say the Witch and her coven steal from us,
wait 1 s
say taking our gold, our produce, and now our children.
wait 1 s
shiver
wait 2 s
say The Baron is helpless, and we are desperate.
wait 2 s
say Pray, help us rid the village of this evil. 
wait 1 s
say Find the witch and slay her. 
wait 1 s
say Bring me her head and I shall reward you!
end
~
#36541
Francois witch question~
0 d 100
witch Tus~
wait 2 s
say I know little of her, save that she lurks in Hillside.
wait 1 s
say Yes, the abandoned cemetary, you must know it...
wait 2 s
mutter
wait 2 s
say Damn that bitch! Were I 20 years younger...
wait 1 s
growl
end
~
#36542
Seer greet~
0 g 100
~
wait 2 s
say Traveller of the world, seeker of fates, weaver of destinies.
wait 1 s
say Perils grave do face thee, heed the warnings of Anger!
wait 2 s
%echo% &06The Seer gazes deeper in to her crystal ball.
wait 3 s
mutter
wait 2 s
frown
wait 1 s
say You must venture to the dark, to find the light...
wait 2 s
gasp
wait 2 s
say Beware the Witch!
wait 2 s
say the Baron, go see the Baron.
wait 2 s
%echo% &06The Seer turns pale.&00
wait 2 s
gasp
wait 2 s
say No! I cannot say any more...
wait 2 s
say leave me now!
wait 1 s
%echo% &06The Seer waves her hand.&00
wait 1 s
%force% %actor.name% e
end
~
#36543
Niles the butler greet~
0 g 100
~
wait 2 s
%echo% Niles sniffs disdainfully.
if %actor.sex% == female
wait 2 s
say What is it you wish... madam?
else
wait 2 s
say Can I help you... sir?
endif
~
#36544
ask niles to see the baron~
0 d 100
see~
wait 2 s
nod
wait 2 s
say Very well. One moment please...
wait 2 s
open door w
wait 2 s
%echo% Niles glances in, and then turns to face you.
wait 2 s
say He says he is far too busy.
Wait 1 s
close door w
wait 2 s
%echo% &06You have a sneaking suspicion that Niles is lying...&00
end
~
#36545
Niles calls for help~
0 k 33
~
say Oh dear, such barbaric behavior... Guards!
wait 1 s
%echo% One of the baron's guards comes rushing in.
wait 2 s
say Sieze that ruffian!
wait 1 s
point %actor.name%
wait 1 s
%load% mob 36518
%force% baronguard grin
%force% baronguard murder %actor.name%
end
~
#36546
Baron greeting~
0 g 100
~
wait 2 s
%echo% The Baron looks up from his reading as you enter.
wait 1 s
stand
wait 2 s
say Tell me, %actor.name% do you normally barge in uninvited on a representative of the Emperor Maximillian himself?
wait 1 s
sigh
wait 2 s
say Well, these are the times we live in...
wait 2 s
say What do you want, then?
end
~
#36547
Ask baron about necro and witch~
0 d 100
witch necro necromancer trouble~
wait 2 s
say You know, then of the troubles we suffer at the hands of Chaos.
wait 2 s
sigh
wait 2 s
say Yes, the cult of the Necromancer, and his agents like the Witch, have plagued us for months.
wait 2 s
say All my efforts to lay the evil to rest have ended in abject failure.
wait 2 s
say I am at a loss...
wait 2 s
ponder
wait 2 s
say However, if you were agreeable, you could always endeavour to end this evil. 
wait 2 s
say In fact, I am prepared to offer a significant reward, for the defeat of the Necromancer.
wait 2 s
say I would of course require proof.
end
~
#36548
ask baron about proof~
0 d 100
proof~
wait 2 s
think
wait 2 s
say bring me the heart of the Necromancer.
wait 2 s
say and I know that my good friend Francois will pay a handsome reward for the head of the witch.
wait 2 s
say So brave adventurer, do you wish to take on this quest and free our land of this evil?
end
~
#36549
say no to barons quest~
0 d 100
no~
wait 2 s
sigh
wait 2 s
say Well, go right ahead and make yourself at home, then.
wait 2 s
say I have nothing more to offer you and I have more pressing business to attend to.
wait 1 s
sit
end
~
#36550
say yes to Baron quest~
0 d 100
yes~
wait 2 s
say oh thank you %actor.name%
wait 2 s
say I can only offer you my prayers to help you in your task.
wait 2 s
say if you visit Maryah the Seer, in town, she might have a few clues for you.
wait 2 s
say also Francois, the burgomeister, knows of the both these vile characters.
wait 2 s
say God Speed %actor.name%.
end
~
#36551
Alain greeting~
0 g 100
~
wait 2 s  
say Needing something repaired stranger?
wait 2 s   
say or looking to purchase something more personalized?
wait 2 s
%echo% Alain lowers the blade and awaits your reply.
end
~
#36552
ask Alain about necro or witch~
0 d 100
witch necro necromancer~
wait 2 s
say You are talking to the wrong person, friend.
wait 2 s
say if it is information you seek, try the Baron.
wait 2 s
say if it is a special weapon, then I am your man.
end
~
#36553
where is baron~
0 d 100
baron~
if %actor.vnum% < 0
wait 2 s
say You can find the Baron in his manor to the east.
endif
~
#36554
Dragons greeting~
0 g 100
~
Wait 2 s
chuckle
wait 2 s
say Boastful whelp, you consider yourself a 'Dragonslayer'??
wait 2 s
grin
wait 2 s
say Well, step closer %actor.name%, and we shall see what you are made of.
wait 2 s
flex
end
~
#36555
give dragon parchment~
0 j 100
cabal~
if %object.vnum% == 36525
wait 2 s
raise
wait 2 s
say What's this?
wait 2 s
peer %actor.name%
wait 2 s
%echo% Dengraz'ch scans the parchment, then devours it in a single gulp.
mjunk obj 36525
wait 2 s
ponder
wait 2 s
say You don't LOOK like a cabalist...
wait 2 s
frown
wait 2 s
say I think you try to fool Dengraz'ch!
wait 2 s
shrug
wait 2 s
say Very well, prepare thyself!
wait 2 s
%echo% Dengraz'ch traces a design on the wall, and a gateway opens!
%load% obj 36568
else
wait 2 s
say what do I want with this junk!
wait 1 s
junk all
endif
~
#36556
dragons death~
0 f 100
~
%echo% &02A greenish mist fills the cavern, and a strange figure emerges...&00
%load% mob 36520
end
~
#36557
necro tease~
0 n 100
~
wait 5 s
say So, you have managed to best my best servant...
wait 1 s
frown
wait 1 s
%echo% The necromancer glances briefly at the mangled corpse then shrugs.
wait 1 s
say No matter, he can be replaced... As for you..
wait 1 s
chuckle
wait 1 s
say you don't even understand the mistake you have made.
wait 1 s
say but I have plans for you, foolish adventurers...
wait 1 s
cackle
wait 1 s
say meet the brothers of Dengraz'ch!
wait 1 s
%echo% &02The cavern fills once more with the greenish mist, and he vanishes!&00
%load% mob 36539
%load% mob 36539
mgoto 36549
end
~
#36558
kill on load~
0 n 100
~
wait 1 s
dg_cast 'wall of fire'
end
~
#36559
Skeleton guard greet~
0 e 100
falls~
wait 10 s
%echo% The guard steps forth, and speaks in a tongue long forgotten...
wait 2 s
say Stranger, you umbrage the bastion of my master.
wait 2 s
say Leave now afore he is disturbed by your intrusion...
wait 2 s
cackle
end
~
#36560
bribe guard~
0 m 50000
~
if %amount% > 49999
wait 2 s
say Now you're talking!?
wait 2 s
%echo% The guard bites the coins to check they are real, then glances about.
%load% obj 36556
wait 2 s
unlock door
wait 1 s
open door
wait 2 s
say Hurry!
wait 2 s
%echo% The guard hustles you through.
wait 1 s
%force% %actor.name% e
wait 5 s
close door
wait 1 s
lock door
wait 2 s
whistle
wait 2 s
inno
mjunk key
else
wait 1 s
say What is this, your milk money?
wait 1 s
say You can do better than that!
wait 1 s
say I'm going to keep this just for wasting my time!
endif
~
#36561
skeletons death~
0 f 100
~
%echo% &08As the guard perishes, a key slips from his bony grasp...&00
%load% obj 36556
~
#36562
give tail plus money to alain~
0 j 100
tail~
if %object.vnum% == 36557
wait 1 s
say What a disgusting object. What would you possibly...
wait 1 s
say hrm
wait 1 s
say Ah yes... you wish for a weapon from its barbed tip, eh?
wait 2 s
say Well it will cost you at least 20million in gold.
mload mob 36540
mgoto 36549
mpurg self
else
wait 1 s
say nice, but I have no need for this.
drop all
endif
~
#36563
bribe alain2~
0 m 1
~
if %amount% 19999999
wait 2 s
say aha! You will not be dissapointed!
wait 2 s
%echo% Alain takes the lethal tail away, and strange sounds can be heard.
mgoto 36549
mjunk all
%load% obj 36558
mget wyvernstail
wait 5 s
mgoto 36576
%echo% When he returns, he is bearing a rather strange weapon.
wait 2 s
say Here is just what you ordered
wait 1 s
give wyvernstail %actor.name%
wait 1 s
%echo% Alain wipes his hands and shakes his head.
mload mob 36525
mgoto 36549
mpurg self
else
frown
wait 2 s
say to steep a price fer ya?
wait 1 s
shake
wait 2 s
say maybe someday, friend.
wait 1 s
mload obj 36557
give spectail %actor.name%
wait 1 s
say have a good day.
wait 1 s
%echo% Alain goes back to work
mload mob 36525
mgoto 36549
mpurg self
endif
~
#36564
Xanclaw greet~
0 g 100
~
if %actor.is_npc% != 0 || %actor.level% > 50
halt
end
eval phrase %random.3%
if %phrase% == 1
snarl
say Intruders, you dare defile my lair?!?
%echo% &08The wyvern's tail lashes out, dripping acidic venom!&00
%send% %actor.name% The tail strikes you, causing a burning pain!
%echoaround% %actor.name% %actor.name% is struck by the tail, and shrieks as the acid burns him.
%damage% %actor.name% 100
dg_cast 'para' %actor.name%
elseif %phrase% == 2
say This is not your day %actor.name%!
%echo% &08The wyvern's tail lashes out, dripping acidic venom!&00
%send% %actor.name% The tail strikes you, causing a burning pain!
%echoaround% %actor.name% %actor.name% is struck by the tail, and shrieks as the acid burns him.
%damage% %actor.name% 200
dg_cast 'blind' %actor.name%
else
say You just made your last mistake %actor.name%!
%echo% &08The wyvern's tail lashes out, dripping acidic venom!&00
%send% %actor.name% The tail strikes you, causing a burning pain!
%echoaround% %actor.name% %actor.name% is struck by the tail, and shrieks as the acid burns him.
%damage% %actor.name% 200
dg_cast 'poison' %actor.name%
endif
~
#36565
xanclaw fight~
0 k 20
~
if %actor.is_npc% != 0 || %actor.level% > 51
halt
end
eval phrase %random.4%
if %phrase% == 1
%echo% &09The wyvern's tail lashes out, dripping acidic venom!&00
damage %actor.name% 200
dg_cast 'poison' %actor.name%
elseif %phrase% == 2
%echo% &08Xanclaw takes to the air, flying towards the sun.&00
mgoto 36549
wait 10 s
mgoto 36589
wait 1 s
%echo% &08Xanclaw suddenly dives from above!&00
dg_cast 'wall of fire'
wait 1 s
cackle
elseif %phrase% == 3
%echo% &08Xanclaw decides to take a break.&00
wait 1 s
%echo% &08Xanclaw flies off&00
mgoto 36549
wait 5 s
goto 36589
say Breaks over kids!
dg_cast 'wall of fire'
wait 1 s
cackle
else
%echo% &08Xanclaw flies off laughing at you!&00
mgoto 36549
wait 10 s
mgoto 36589
%echo% &08Xanclaw flies in from above.&00
wait 1 s
say I like my hot dogs well done.
dg_cast 'wall of fire'
endif
~
#36566
villager talks~
0 g 100
~
if %actor.vnum% < 0
wait 2 s
say can you please help us find our children?
wait 1 s
sigh
wait 2 s
say talk to the Baron, please.
endif
~
#36567
villager talks~
0 g 100
~
if %actor.vnum% < 0
wait 2 s
sigh
wait 1 s
say can you please help us?
wait 1 s
sigh
wait 2 s
say talk to the Baron, please.
endif
~
#36568
trapdoor pull~
2 c 100
pull~
if %actor.vnum% < 0
%echoaround% %actor% You hear a crack as %actor.name% pulls on the trap door.
%send% %actor% You hear click as you pull on the trap door.
%echo% The trap door springs open reveling a passage down.
wdoor 36583 down room 36584
wdoor 36584 up room 36583
wait 3 s
%echo% The trap door slams back into place covering the passage.
wdoor 36584 up purg
wdoor 36583 down purg
endif
~
#36569
purg trap door~
2 g 100
~
wait 2 s
wdoor 36584 up purg
wdoor 36583 down purg
%echo% The trap door slams back into place covering the passage.
end
~
#36570
xanclaw death load~
0 f 100
~
%load% obj 36557
end
~
#36571
tus greet~
0 ag 100
~
if %alreadyrun4% == Yes
halt
end
wait 2 s
giggle
wait 2 s
say Adventurers... how charming!
wait 2 s
snicker
wait 2 s
ponder
wait 2 s
say Tell me, %actor.name%, before I eat your heart, would you prefer to be slaughtered by knife or claw?
wait 2 s
cackle
wait 1 s
dg_cast 'pest'
eval alreadyrun4 Yes
global alreadyrun4
~
#36572
hole in wall~
2 d 100
curses~
if %actor.vnum% > 0
wait 10 s
%echo% &01As you pull your weapon from the blubbery flesh of this creature, a rip appears.&00
wait 2 s
wdoor 36596 north room 36597
wdoor 36597 south room 36596
%echo% &01Slowly the gap widens and a gaping hole forms through the blubbery wall.&00
wait 50 s
%echo% &01The rip in the wall of flesh begins to shift and shake violently.&00
wait 40 s
%echo% &01Large pools of a puss like substance begins bubbling from the wall of flesh at an alarming rate.&00
wait 30 s
%echo% &01The hole through the wall slowly begins to close up with a disturbing slurping sound.&00
wdoor 36596 north purg
wdoor 36597 south purg
endif
~
#36573
wall dies~
0 l 6
~
wait 1 s
say may a thousand curses be placed on your soul!
end
~
#36574
necro call helper~
0 l 10
~
Wait 2 s
%echo% &08The Necromancer makes a gesture, and summons a ghoulish minion from the Abyss!&00
%load% mob 36528
%force% ghoulish assist necromancer
end
~
#36575
necro heart~
0 f 100
~
%echo% &08With a horrid ripping sound, his heart explodes from his chest,&00
%load% obj 36565
drop heart
end
~
#36576
give heart to baron~
0 j 100
heart~
if %object.vnum% == 36565
wait 2 s
gasp
wait 1 s
say You... you have succeeded where all others failed, %actor.name%.
wait 2 s
say It is an honor to have you before me, you have saved us all.
wait 1 s
say Words cannot express the gratitude I feel.
wait 2 s
%echo% &13At this moment, an envoy of the Emperor enters the room, and approaches.&00
wait 1 s
%echo% &13Whispering in the Baron's ear, he hands him a small package.&00
wait 2 s
%echo% &13The envoy leaves then, without acknowledging your presence.&00
wait 1 s
smile
wait 1 s
say The Seer-mages of the Emperor informed him of your achievement.
wait 2 s
say In gratitude, he had them transport hither this item...
wait 2 s
%echo% &10A brilliant glow suffuses the chamber as the Baron opens the package.&00
wait 1 s
say %actor.name%, I present thee with the Medal of Glory!
wait 2 s
say Wear it with pride...
%load% obj 36566
wait 2 s
give medal %actor.name%
mjunk all
else
wait 1 s
say I have no need of this.
wait 1 s
drop all
endif
~
#36577
give head to burger~
0 j 100
head~
if %object.vnum% == 36569
wait 1 s
gasp
wait 2 s
say You slew the foul witch, Tus'schepteba!
wait 2 s
say O Fine Day!
wait 2 s
say This is incredible... I have no way of expressing my joy.
wait 2 s
say We have only this small reward to give you.
%load% obj 36570
give reward %actor.name%
wait 1 s
say Thank you again %actor.name%, we are forever in your debt.
wait 1 s
bow
mjunk all
else
wait 1 s
say I have no need of this item.
wait 1 s
drop all
endif
~
#36578
tus death load head~
0 f 100
~
%load% obj 36569
end
~
#36579
where chapel key~
0 d 100
chapel key~
wait 2 s
ponder
wait 2 s
say The ole' chapel key was lost amoungst the graves many a year ago.
wait 2 s
say I hear tell of skeletons and ghouls wandering around in there.
wait 2 s
say not a place you would catch me.
end
~
#36580
help baron Niles~
0 d 100
help baron~
wait 2 s
say You?
wait 1 s
say You are here to help the Baron?
wait 2 s
say Well if you insist.
wait 1 s
open door w
Wait 2 s
say %actor.name% to see you Baron.
wait 2 s
say well hurry along, don't keep him waiting!
wait 1 s
mforce %actor.name% w
wait 2 s
close door w
end
~
#36581
alain weapon speech~
0 d 100
buy create special~
wait 1 s
say if ya want a special weapon, you will have to bring me something special to make it from.
wait 2 s
say and it won't be cheap!
wait 2 s
say so be on your way, till you have something to offer.
end
~
#36582
G'gor key~
0 f 100
~
%load% obj 36537
end
~
#36583
restore on flee~
0 e 100
flee!~
wait 1 s
mgoto %actor.name%
mkill %actor.name%
end
~
#36584
mm test~
0 e 100
'kariq wugguro'.~
wait 1 s
mgoto %actor.name%
mkill %actor.name%
end
~
#36585
throw test~
0 e 100
You take a~
wait 1 s
dg_cast 'revive'
say ok
end
~
#36586
give orb~
1 i 100
~
if %victim.vnum% < 0
if %victim.class% != knight
%send% %victim.name% The orb zaps you with a bolt of white fire.
%echoaround% %victim.name% The orb emits a bolt of white fire as it burns %victim.name%.
%damage% %victim.name% 1
return 0
endif
endif
~
#36587
get orb~
1 g 100
~
if %actor.vnum% < 0
if %actor.class% != knight
%send% %actor.name% The orb zaps you with a bolt of white fire.
%echoaround% %actor.name% The orb emits a bolt of white fire as it burns
%actor.name%.
%damage% %actor.name% 100
return 0
endif
endif
~
#36588
mage super aggro doombolt attached to mortus beast~
0 g 100
~
if %actor.is_npc% != 0 || %actor.level% > 50
halt
else
dg_cast 'doom bolt' %actor%
end
~
#36589
calm killer~
0 c 100
calm~
wait 3
say That won't work here.
kill %actor.name%
end
~
#36595
Living wall trigger~
0 f 100
~
%echo% &08A gaping hole is hacked through the blubbery wall.&00
end
~
#36596
Living wall open and seal trigger2~
2 c 100
pull~
%echo% &01As you pull at the tear caused by your recent battle, you have goblets of flesh dribbling down your arms and covering yourself in gore.&00
wait 2 s
wdoor 36596 north room 36597
%echo% &01Slowly the gap widens and a gaping hole forms through the blubbery wall.&00
wait 30 s
%echo% &01The hole through the wall slowly begins to close up with a disturbing slurping sound.&00
wdoor 36596 north purg
end
~
#36597
Run Once Greet Trigger~
0 ag 100
~
if %alreadyrun% == Yes
   halt
end
say hello
eval alreadyrun Yes
global alreadyrun
~
#36598
anti palm object --> o36521~
1 cg 100
palm~
if %actor.class% == thief
%damage% %actor.name% 100
%send% %actor.name% You recieve a nasty burn to your hand.
%echoaround% %actor.name% %actor.name% receives a nasty burn to his hand.
end
~
#36599
knight only simple test~
1 g 100
~
if %actor.vnum% < 0
if %actor.class% != Solamnic Knight
%send% %actor.name% The orb zaps you with a bolt of white fire.
%echoaround% %actor.name% The orb emits a bolt of white fire as it burns %actor.name%.
%damage% %actor.name% 100
return 0
endif
endif
~
$~
