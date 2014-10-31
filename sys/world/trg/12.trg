#1201
Imhotep's Cuckoo Clock~
0 gh 15
~
wait 3
%echo% John Kerry says, 'Hi! I'm John Kerry, and I approved this message!'
wait 5
%echo% John Kerry says, 'Vote John Kerry, for a better life!'
~
#1202
Inhotep's Word of Closing~
2 d 100
close exits~
if %speech% != close exits
halt
end
if 0 != Z
%echo% 0 != Z = 1
end
%echo% The walls blur briefly, then solidify, closing the views to the outside
%echo% world.
wdoor 1280 west purge
wdoor 1280 east purge
wdoor 1280 south purge
wdoor 1280 up purge
~
#1203
Imhotep's Word of Power~
2 d 100
friend~
if %speech% != friend
halt
end
if %actor.name% == Imhotep
  %echo% At your word of command, the walls of the chamber shimmer briefly, then   
  %echo% &00quickly fade to translucency.  Beyond, various scenes from around the
  %echo% &00world become visible.
  wdoor 1280 west room 18001^M
  wdoor 1280 east room 22286^M
  wdoor 1280 south room 32475^M
  wdoor 1280 up room 18167^M
  wait 60
  wdoor 1280 west purge
  wdoor 1280 east purge
  wdoor 1280 south purge
  wdoor 1280 up purge
  %echo% The walls of the chamber shimmer briefly, then return to opacity.
else
  %echo% At your word of command, the walls of the chamber shimmer briefly, but then
  %echo% &00turn a dull red colour, before slowly returning to their original state.
 end
~
#1204
Panic!~
0 g 40
~
if (%actor.is_npc% != 0 || %actor.level% > 50)
         halt
      else
flee
end
~
#1205
Orcus quest~
0 e 100
"nods in agreement with you."~
wait 1 s
say You must help me!  Orcus has escaped from the Astral planes and is wreaking havoc!
wait 3 s
say The last I saw he was in the Kingdom of Juargan, massacring the dwarves!
wait 2 s
say Please, brave adventurer, save the land from this terror!
~
#1206
orcus quest #2~
0 b 90
~
hit dwarf
~
#1207
command script~
2 c 100
friend~
%echo% Password spoken
~
#1208
Timer test~
1 f 100
~
%send% %owner% The sand in the hourglass fills one side, and the glass flips in your hand.
otimer 1
~
#1209
Timer Attach Script~
1 j 100
~
eval owner %actor%
global owner
attach 1208 %self.id%
wait 1
%send% %owner% Sands start pouring through the hourglass!
otimer 1
~
#1210
Timer Detach Script~
1 l 100
~
detach 1208 %self.id%
otimer 0
wait 1
%send% %owner% The sand in the hourglass suddenly freezes!
~
#1211
Dikfer testing~
0 abd 25
go~
eval oldroom %self.room%
say It's thievin' time!
mgoto 18001
%echo% A tiny dikfer has arrived.
eval victim %random.char%
mgoto %oldroom%
if %victim.is_npc% != 0 || %victim.level% > 50
  mgoto %oldroom%
  say Oops, that wasn't a good target.
else
*%send% %victim% You discover that a tiny dikfer has its hands in your wallet.
*%echoaround% %victim% A tiny dikfer tries to steal gold from %victim.name%.
mat 18001 steal gold %victim.name%
%echo% waiting
wait 50
mat 18001 flee
mgoto %oldroom%
say I victimized %victim.name%.
end
~
#1212
Exploding Trap (200)~
1 g 100
invoke~
%echoaround% %actor% Perhaps %actor.name% shouldn't have messed with %self.shortdesc%.
%send% %actor% Uh oh... You shouldn't have messed with that.
%echo% &08%self.shortdesc% explodes with a blinding flash&00
%echo% &08A wave of heat washes over you&00
%damage% %actor% 200
return 0
%purge% %self%
~
#1213
Igor halloween quest trigger~
0 l 10
~
%load% mob 1291
eval room %self.room%
eval newmob %room.people%
%force% %newmob% mhunt %actor%
%echo% Igor is dead!  R.I.P.
%purge% %self%
~
#1214
igor's ghost reward~
0 n 100
~
eval chance %random.12%
   switch %chance%
     case 1
      %load% obj 1655
      take pot
      break
     case 2
      %load% obj 85
      take pot
      break
     case 3
      %load% obj 1650
      take candy
      break
     case 4
      %load% obj 200
      take potion
      break
     case 5
      %load% obj 84
      take potion
      break
     case 6
      %load% obj 1412
      take candy
      break
     case 7
      %load% obj 23222
      take coffee
      break 
     case 8
      %load% obj 84
      take potion
      break
     case 9
      %load% obj 84
      take potion
      break
     case 10
      %load% obj 1412
      take candy
     break
     case 11
      %load% obj 1412
      take candy
      break
     case 12
      %load% obj 1655
      take
      break
     default
      %echo% error in igor's ghost's halloween quest trigger.  Tell an Imm asap.
      done
~
#1215
Baseball beta~
0 b 5
~
%echo% A baseball comes flying in at extremely high speed from above.
eval chance %random.4%
switch %chance%
case 1
mgoto 1280 
dg_cast 'teleport' %self.name%
%load% obj 1283
drop baseball
mgoto 18001
%echo% The baseball player hits a baseball with a mighty WHACK!
break
case 2
%echo% The baseball player swings at the baseball, but misses with a woosh of air.
say Damnit!
swear
break
case 3
%echo% The baseball player oofs as the ball comes too close to his head and bursts into flames.
shout What are you trying to do? Kill me? 
break
default
%echo% The baseball player hits the baseball so hard that it disintegrates.
say oops...  they just don't make baseballs like they used to.
break
~
#1216
aggro-wield~
0 g 100
~
if %actor.eq(19)%
say So it's a challenge that you want?  It's a challenge that you will get!
mkill %actor%
else
halt
end
~
#1217
Statue to life~
1 c 100
l~
if %cmd.mudcommand% == look
  if %self.name% /= %arg% && %arg.strlen% > 0
   %echo% %self.shortdesc% seems to come alive!
   %load% mob %self.vnum%
   %purge% %self%
  else
   return 0
   halt 
  end
else
return 0
end
~
#1218
You talkin to me~
0 d 1
shutup ugly bastard jerk dork asshole idiot~
say You talkin to me, %actor.name%?
mhunt %actor%
~
#1219
Magic 8-ball~
1 h 100
~
return 0
   %echoaround% %actor% %actor.name% shakes up %self.shortdesc%, and turns it over. 
   %send% %actor% You shake up %self.shortdesc% and turn it over.
   eval chance %random.19%
   switch %chance%
    case 1
     %echo% %self.shortdesc% says, 'Ask again later.'
     break
    case 2
     %echo% %self.shortdesc% says, 'Not even on your birthday.'
     break
    case 3
     %echo% %self.shortdesc% says, ' No'
     break
    case 4
     %echo% %self.shortdesc% says, ' Yes'
     break
    case 5
     %echo% %self.shortdesc% says, ' Only if you manage to get a date before you retire.'
     break
    case 6
     %echo% %self.shortdesc% says, ' Dream on.'
     break
    case 7
     %echo% %self.shortdesc% says, ' If I say yes, will you stop asking me?'
     break
    case 8
     %echo% %self.shortdesc% says, ' Maybe'
     break
    case 9
     %echo% %self.shortdesc% says, ' Sure, why not, stranger things have happened.'
     break
    case 10
     %echo% %self.shortdesc% says, 'My sources say no.'
     break
    case 11
     %echo% %self.shortdesc% says, 'I suggest killing Regis instead.'
     break
    case 12
     %echo% %self.shortdesc% says, ' Most Likely'
     break
    case 13
     %echo% %self.shortdesc% says, ' ! '
     break
    case 14
     %echo% %self.shortdesc% says, ' To tell you the truth, I have absolutely no frickin clue.'
     break
    case 15
     %echo% %self.shortdesc% says, ' Did Mars marry a gnome?'
     break
    case 16
     %echo% %self.shortdesc% says, ' You're kidding, right?'
     break
    case 17
     %echo% %self.shortdesc% says, ' Not even a half-melted snowball's chance in Avernus.'
     break
    case 18
     %echo% %self.shortdesc% says, ' uh..... huh huhuhuh...  that was cool.'
     break
    default
     %echo% %self.shortdesc% says, 'Please record your question after the beep, and I will get back to you.'
     break
~
#1220
Lodestone~
1 h 100
~
   return 0
%send% %actor% You try to drop %self.shortdesc% but it's chain is wrapped around your waist.
%echoaround% %actor% %actor.name% tries to get rid of the lodestone, but can't seem to detach the chain.
end
~
#1221
test-memnoch~
0 k 100
~
detach mob %self.id% 1
~
#1233
anubis's room trigger~
2 c 100
gimme~
dg_cast 'sanct' %actor%
dg_cast 'shield' %actor%
dg_cast 'stoneskin' %actor%   
dg_cast 'blur' %actor%
dg_cast 'pulse heal' %actor%
dg_cast 'pulse gain' %actor%   
dg_cast 'cleanse' %actor%
dg_cast 'fly' %actor%
dg_cast 'regenerate' %actor%
dg_cast 'shadow vision' %actor%   
dg_cast 'true sight' %actor%
dg_cast 'haste' %actor%
dg_cast 'flame blade' %actor%  
dg_cast 'bless' %actor%
dg_cast 'air' %actor%
dg_cast 'shadow walk' %actor%
~
#1234
shrapnel grenade~
1 h 100
~
* This script was made to test cycling through everyone in the room
set roomvar %actor.room%
wait 15
%echo% The grenade explodes in a flash of light and smoke.
* Next line sets the first person in the room to targetchar
set targetchar %roomvar.people%
while %targetchar%
* In case the current target gets killed by the blast, set the next person in line now
set tmp %targetchar.next_in_room%
%send% %targetchar% &01Shrapnel from the explosion tears into your flesh.&00
eval inflict %random.300% +100
%damage% %targetchar% %inflict%
set targetchar %tmp%
done
%purge% %self%
~
#1235
roomsquaker~
2 b 10
~
eval phrase %random.5%
   if %phrase% ==1
     %echo% The starlight dances on the black marble floor as the stars overhead twinkle.
   elseif %phrase% ==2
     %echo% A light breeze rustles the leaves of the trees here.
   elseif %phrase% ==3
%echo% A flying squirrel &25jumps&00 from one treetop to the next.
   elseif %phrase% ==4
     %echo% A dark shape eclipses the stars and moons overhead.
   else
     %echo% Lovely fragrances of flowers and nature linger in the air.
   end
~
#1236
random test trigger~
0 b 13
~
%echo% Random script fired
~
#1237
Clan Recall~
1 c 100
click~
if %actor.clan% != 1
  %send% %actor% Ducan McLeod appears before you, and cuts your head off.
  %damage% %actor% 5000
  halt
end 
%send% %actor% You click your fingers twice sharply.
%send% %actor% Your vision blurs and distorts, then clears
%teleport% %actor% 1298
%force% %actor% look
%echo% %actor.name% phases out of existence.
return 0
~
#1240
Spreading Flames~
0 f 100
~
%echo% %self.name% flickers a moment, then spreads to another spot on the floor.
%load% mob %self.vnum%
%echo% %self.name% burns brightly for a moment.
~
#1242
Kenny Trigger~
0 e 0
Kenny is dead!  R.I.P.~
shout You bastards! You killed Kenny!
~
#1250
Room trigger command test~
2 c 100
Trebuchet~
%echo% actor is %actor.name%
~
#1251
Sun Tzu~
0 k 15
~
say I am going to release my trebuchet on you!!!!
trebuchet %actor.name%
mecho The trebuchets are lauched - you can hear them in the distance!
~
#1252
antilanthulanu m1250 attack only 50th level trigger~
0 h 100
~
if %actor.is_npc% != 0 || %actor.level% < 50
halt
else
kill %actor.name%
end
~
#1278
Questrabbit announcement~
0 d 100
quest~
if %actor.level% < 51 && %actor.name%
  halt
end
if %speech.car% != quest || %speech.strlen% != 5
  halt
end
eval room %self.room%
ooc Find me in %room.name% before I leave and receive a prize!
attach 1279 %self.id%
~
#1279
Questrabbit prize~
0 g 100
~
if %actor.is_npc% == 0 && %actor.name% && %actor.level% < 51
  ooc %actor.name% has found me!
  say Well done!  Your reward is two quest points!
  %echo% The rabbit vanishes suddenly!
  mgoto imhotep
end
~
#1280
Tormentor's Script~
0 d 100
peace~
if %speech% != peace
halt
end
if %actor.level% < 51 && %actor.name%
  say Hah, you think you can command ME?
  halt
end
eval hunted digger
global hunted
say I'll just sit here and mind my own business, then
~
#1281
Tormentor's Attach~
0 d 100
hunt~
if %actor.level% < 51 && %actor.name%
  say Hah, you think you can command ME?
  halt
end
if %speech.car% != hunt || %speech.strlen% < 7
  say Try "hunt <victim>"
  halt
end
eval hunted %speech.cdr%
if %hunted.car% != %hunted%
  say I can only hunt one thing at a time!
  halt
end
if %hunted.is_npc% != 0
  say I can't hunt NPCs!
  halt
end
if %hunted.level% > 50
  say Are you crazy?  That's a GOD!
  halt
end
if %hunted.room%
  global hunted
  say Ok, I'm hunting %hunted%
else
  say That person isn't even logged on!
end
~
#1282
Tormentor's Torments~
0 ab 45
~
if %hunted% == digger
  halt
end
if !%hunted.room%
%echo% The tormentor looks crestfallen as its victim escapes it.
eval hunted digger
global hunted
halt
end
if %hunted.room% != %self.room%
  %echo% The tormentor demon scratches a small hole in reality, and pops through it,
  %echo% &00head first.
  mgoto %hunted%
  %echo% A small hole in the fabric of reality opens up, and a small demon pokes its
  %echo% &00head through.  It looks around, spots %hunted%, smiles and forces itself out
  %echo% &00of the hole, which closes behind it.
  halt
end
eval option %random.9%
if %option% == 1
  %echo% The tormentor demon squints.
  dg_cast 'blind' %hunted%
elseif %option% == 2
  %echo% The tormentor demon mutters to itself.
  dg_cast 'slow' %hunted%
elseif %option% == 3
  %echo% The tormentor demon looks around confusedly.
  dg_cast 'teleport' %hunted%
elseif %option% == 4
  %echoaround% %hunted% The tormentor demon swiftly moves in and bites %hunted%!
  %send% %hunted% The tormentor demon swiftly moves in and bites you, hard!
  dg_cast 'poison' %hunted%
elseif %option% == 5
%echo% The tormentor demon puts a finger to its lips and says, 'Shh!'
  dg_cast 'silence' %hunted%
elseif %option% == 6
  %echoaround% %hunted% The tormentor demon jams a tentacle into %hunted%!
  %send% %hunted% The tormentor demon jams a tentacle into your abdomen!
  dg_cast 'energy drain' %hunted%
elseif %option% == 7
  %echoaround% %hunted% The tormentor sneezes on %hunted%!
  %send% %hunted% The tormentor demon crinkles up its nose and sneezes on you!
  dg_cast 'plague' %hunted%
elseif %option% == 8
  %echoaround% %hunted% The tormentor looks at %hunted% with a confused face.  
  %send% %hunted% The tormentor demon contorts its face into chaos!
  dg_cast 'feeblemind' %hunted%
elseif %option% == 9
  %echoaround% %hunted% The tormentor pricks its finger and flicks a drop of blood at %hunted%!
  %send% %hunted% The tormentor demon flicks a drop of blood into your mouth!
  dg_cast 'debilitate' %hunted%
end
~
#1283
Cheat KILLER!~
0 n 100
~
say Now you must suffer!
wait 5s
mgoto 3032
say Now you must suffer!
~
#1284
Throw targetter~
2 d 100
target~
extract target 2 %speech%
if %target% && %target.room%
    %echo% Opening a throwing portal to %target%
    %door% 1280 down room %target.room%
    wait 2s
    %echo% Closing portal.
    %door% 1280 down purge
else
    %echo% Cannot find target!
end 
~
#1285
teleport trigger~
1 j 100
~
%teleport% all 1280
%force% %actor% look
return 0
~
#1286
Donation Fairy~
0 d 100
deliver~
   if %actor.level% < 50 && %actor.name%
     halt
   else
     say Oooooooh! More goodies for my newbies!
     cackle
     wait 2s
     say I'm off to drop this stuff off!
     s
     wait 2s
     e
     wait 2s
     d
     wait 2s
     e
     wait 2s
     drop all
     wait 2s
     shout Phew! that was a heavy load!
     wait 2s
     w
     wait 2s
     u
     wait 2s
     w
     wait 2s
     n
     say man, that was really friggin heavy...
   end
~
#1287
Quest cube of power loader~
0 f 100
~
%load% obj 18199
%echo% As the guardian drops to the ground, a light flashes from within his cloak. 
return 0
~
#1288
Mob mover~
0 n 100
~
eval dirn %random.6%
switch %dirn%
  case 1
   %echo% north
   north
   break
  case 2
   %echo% south
   south
   break
  case 3
   %echo% east
   east
   break
  case 4
   %echo% west
   west
   break
  case 5
   %echo% up
   up
   break
  case 6
   %echo% down
   down
   break
  default
   %echo% error in halloween Tell Caleb asap.
   done
~
#1290
Yo Yo!~
1 i 100
~
return 0
if %actor.level% > 50
%echoaround% %actor% %actor.name% flings %self.shortdesc% at %victim.name%'s head.
%send% %actor% You fling %self.shortdesc% at %victim.name%'s head.
%echoaround% %actor% %self.shortdesc% tags %victim.name% in the forehead, and zings back into %actor.name%'s hand on a small string.
%send% %actor% %self.shortdesc% tags %victim.name% in the forehead and zings back into your hand on a small string.
%damage% %victim% %random.30%
else
%echoaround% %actor% %actor.name% flings %self.shortdesc% at %victim.name%'s head.
%send% %actor% You fling %self.shortdesc% at %victim.name%'s head.
%echoaround% %actor% %self.shortdesc% stops short of %victim.name%'s  forehead, and zings back hitting %actor.name% in the face.
%damage% %actor% 1
%send% %actor% %self.shortdesc% stops short of %victim.name%'s forehead and zings back to hit you in the face.
end
~
#1291
Memnoch Testing~
1 h 100
~
return 0
%echo% %actor.name% drops %self.shortdesc%.
%echo% %self.shortdesc% returns to %actor.name%'s hand on a small string.
~
#1292
were-wolf~
0 k 80
~
eval werevnum 1285
* Change the above vnum to the mob you want to transform to
eval transformto &08The %self.name% grows rapidly into a werewolf.&00
eval transformfrom &14The werewolf blurs, and returns to it's natural form.&00
* Chanage the transformto and from variables to the text you want displayed on each transform
* do not change anything below this line
eval original %self.vnum%
%echo% %transformto%
mtransform 1285
wait 300
%echo% %transformfrom%
mtransform %original%
~
#1293
Imm-npc-targ verify~
0 g 100
~
eval victim %actor%
if %victim.is_npc% != 0 || %victim.level% > 50
  %echo% %victim.name% is an immortal, or NPC
else
 %echo% %victim.name% is a valid target%
end
~
#1294
Yoyo Walk the dog~
1 j 100
~
%echo% %actor.name% drops %self.shortdesc%.
%echo% %self.shortdesc% hangs on the end of it string spinning madly.
%echo% %self.shortdesc% returns to %actor.name%'s hand on a small string.
~
#1295
Around the World~
1 l 100
~
%echo% %actor.name% drops %self.shortdesc%.
%echo% %self.shortdesc% swings around in a wide arc, blurring in the air.
%echo% %self.shortdesc% returns to %actor.name%'s hand on a small string.
~
#1296
Cloning Trigger~
0 k 35
~
if %count% && %count% > 2
halt
end
eval count %count% + 1
global count
%echo% %self.name% shivers a moment, then splits into two identical clones.
%load% mob %self.vnum%
eval room %self.room%
eval newmob %room.people%
if %random.100% > 70
attach 1296 %newmob.id%
end
%force% %newmob% mhunt %actor%
~
#1297
Clone death trigger~
0 f 100
~
eval clones %actor.clones% - 1
if %clones% <= 0
halt
end
remote clones %actor.id%
~
#1298
Clanhallb~
2 c 100
callbaal~
if %actor.clan% != 1
 %send% %actor% The Soul of Baal appears before you, thrusting his hand in to your chest,
 %send% %actor% he rips out your heart, showing it to you.
 %damage% %actor% 5000
 halt
end
%send% %actor% You whisper softly to yourself three times, Baal Baal Baal.
%send% %actor% Your molecules begin to shift.
%teleport% %actor% 1215
%force% %actor% look
%echo% %actor.name% phases out of existence.
~
#1299
Clanhall~
2 c 100
click~
if %actor.clan% != 1
  %send% %actor% Ducan McLeod appears before you, and cuts your head off.
  %damage% %actor% 5000
  halt
end 
%send% %actor% You click your fingers twice sharply.
%send% %actor% Your vision blurs and distorts, then clears
%teleport% %actor% 1298
%force% %actor% look
%echo% %actor.name% phases out of existence.
~
$~
