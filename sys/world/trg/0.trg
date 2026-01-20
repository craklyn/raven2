#0
hasana testing trigger~
0 g 10
~
say Happy Valentine's Day to you!
drop heart
~
#1
memory test trigger~
0 o 100
~
* assign this to a mob, force the mob to mremember you, then enter the
* room the mob is in while visible (not via goto)
say I remember you, %actor.name%!
~
#2
mob greet test~
0 g 100
~
if %direction%
  say Hello, %actor.name%, how are things to the %direction%?
else
* if the character popped in (word of recall, etc) this will be hit
  say Where did YOU come from, %actor.name%?
end
~
#3
obj get test~
1 g 100
~
%echo% You hear, 'Please put me down, %actor.name%'
~
#4
room test~
2 g 100
~
wait 50
wsend %actor% you enter a room
~
#5
car/cdr test~
0 d 100
test~
say speech: %speech%
say car: %speech.car%
say cdr: %speech.cdr%
~
#6
subfield test~
0 c 100
test~
* test to make sure %actor.skill(skillname)% works
say your hide ability is %actor.skill(hide)% percent.
*
* make sure %actor.eq(name)% works too
eval headgear %actor.eq(head)%
if %headgear%
  say You have some sort of helmet on
else
  say Where's your headgear?
  halt
end
say Fix your %headgear.name%
~
#7
object otransform test~
1 jl 7
test~
* test of object transformation (and remove trigger)
* test is designed for objects 3020 and 3021
* assign the trigger then wear/remove the item
* repeatedly.
%echo% Beginning object transform.
if %self.vnum% == 3020
  otransform 3021
else
  otransform 3020
end
%echo% Transform complete.
~
#8
makeuid and remote testing~
2 c 100
test~
* makeuid test ---- assuming your MOBOBJ_ID_BASE is 200000,
* this will display the names of the first 10 mobs loaded on your MUD,
* if they are still around.
eval counter 0
while (%counter% < 10)
  makeuid mob 200000+%counter%
  %echo% #%counter%      %mob.id%   %mob.name%
  eval counter %counter% + 1
done
%echoaround% %actor% %actor.name% cannot see this line.
*
*
* this will also serve as a test of getting a remote mob's globals.
* we know that puff, when initially loaded, is id 200000. We'll use remote
* to give her a global, then %mob.globalname% to read it.
makeuid mob 200000
eval globalname 12345
remote globalname %mob.id%
%echo% %mob.name%'s "globalname" value is %mob.globalname%
~
#9
mtransform test~
0 g 100
~
* mtransform test
* as a greet trigger, entering the room will cause
* the mob this is attached to, to toggle between mob 1 and 99.
%echo% Beginning transform.
if %self.vnum%==1
  mtransform -99
else
  mtransform -1
end
%echo% Transform complete.
~
#10
attach test~
0 d 100
attach~
attach 9 %self.id%
~
#11
attach test~
0 d 100
detach~
detach 9 %self.id%
~
#12
Mayoral Greet trigger~
0 b 3
~
' Welcome to Samsera
~
#13
Arrow Storm Suffer Room~
2 b 100
~
if %self.people%
   eval victim %random.char%
   if %victim.level% > 50
    halt
   end
   if %victim.is_npc% != 0
    halt
   end  
   %echo% A storm of arrows sweeps across the ground.
   %damage% %victim% %random.100%
   %echoaround% %victim% Several arrows sprout from %victim.name%'s chest.
   %send% %victim% You take several arrows in the chest.
else
  halt
~
#14
Cold Suffer Room~
2 b 60
~
dg_cast 'ice storm'
~
#15
Earthquake Suffer Room~
2 b 60
~
dg_cast 'earthquake'
~
#16
Pestilence Suffer Room~
2 b 60
~
dg_cast 'pestilence'
~
#17
Lightning Suffer room~
2 b 60
~
dg_cast 'chain lightning'
~
#18
Sacred Ground~
2 b 60
~
dg_cast 'holy word'
~
#19
Unholy ground~
2 b 40
~
dg_cast 'unholy word'
~
#20
Gate opener #1~
0 m 25000
~
eval keynum %self.eq(20)%
eval keynum %keynum.vnum%
bow
wait 2
rem key
unlock gate
open gate
wait 2
say A pleasure doing business with you.
wait 10s
close gate
lock gate
mjunk key
%load% obj %keynum%
hold key
~
#21
Gate opener #2~
0 m 0
~
wait 1 s
scoff
wait 1 s
wait 2 s
say what is this, your milk money?
wait 1 s
say you are going to have to better than that, cheapskate!
end
~
#22
say test trigger, dont use~
0 f 100
~
say test
~
#23
dragon para tank trigger~
0 k 12
~
%echo% A dragon booms,  "I can see you and your every weakness."
      wait 10
%echoaround% %actor% The wandering Dragon slices into the spine of %actor.name%.
%send% %actor% The wandering Dragon slices into your spine with it's razor-sharp claws.
dg_cast 'paralyze' %actor.name%
dg_cast 'paralyze' %actor.name%
dg_cast 'paralyze' %actor.name%
dg_cast 'paralyze' %actor.name%
dg_cast 'paralyze' %actor.name%
~
#24
Mob attack when casted on~
0 e 0
stares at you and utters the words,~
   if %actor.is_npc% != 0 || %actor.level% > 50
   halt
   else
   wait 1
   %echo% The wandering dragon turns and RRRROOOOAAAARRRRSSSS at %actor.name%
   dg_cast 'doom' %actor.name%
   end
~
#38
missing trigger 3~
2 g 100
~
say this is a missing trigger please contact an imm thank you
~
#42
quest mob test trigger~
0 f 100
~
%echo%A pillar of white light extends from the heavens, burning the corpse of %self.name%
and leaving behind a pile of ashe.
%load% obj 1273
%purge% Antilanthulanu
~
#51
missing trigger~
2 g 100
~
say this is a missing trigger please contact an imm thank you
~
#55
missing trigger~
2 g 100
~
say this is a missing trigger please contact an imm thank you
~
#60
SuperAggro (All Classes)~
0 h 100
~
* Super Aggro script that detects mob class and performs accordingly
   eval class %self.class%
   eval level %self.level%
   eval align %self.align%
   eval victim %actor%
if %victim.is_npc% != 0 || %victim.level% > 50
     %send% %self% %victim.name% is an immortal, or NPC
      halt
   else
     %send% %self% %victim.name% is a valid target
     switch %class%
       case Warrior
           * Do what I do best
           if %self.eq(19)%
             bash %actor.name%
           else
             kill %actor.name% 
             kick %actor.name%
           end
         break
       case Ranger
           * Do what I do best
         trip %actor.name%
         break
       case Shou-Lin
           * Do what I do best
         if %level% < 12
           kill %actor.name%
         elseif %level% >= 12 && %level% < 18
           sweep %actor.name%
         elseif %level% >= 18 && %level% < 23
           blind %actor.name%
         elseif %level% >= 23 && %level% < 33
           knock %actor.name%
         else
           fist %actor.name%
         end
         break
       case Cleric
           * Do what I do best
         if %level% >= 43 && %align% < -250
           dg_cast 'unholy word'
         elseif %level% >= 43 && %align% > 250
           dg_cast 'holy word'
         else 
           dg_cast 'earthquake'
         end
         break
       case Solamnic Knight
           * Do what I do best
         if %level% < 9
           kill %actor.name%
         elseif %level% >= 9 && %level% < 44 && %self.eq(19)%
           bash %actor.name%
         elseif %level% > 44 && %actor.align% < -250
           dg_cast 'holy word'
         else 
           kick %actor.name%
         end
         break
       case Death Knight
           * Do what I do best
         if %level% < 9
           kill %actor.name%
         elseif %level% >= 9 && %level% < 30 && %self.eq(19)%
           bash %actor.name%
         elseif %level% >= 30 && $level% < 45
           dg_cast 'pestilence'
         elseif %level% >= 45 && %actor.align% > 250
           dg_cast 'unholy word'
         else 
           kick %actor.name%
         end
         break
       case Shadowdancer
           * Do what I do best
         if %level% < 10 && %self.eq(19)%
           backstab %actor.name%
         elseif %level% >=10 && %level% < 40
           dg_cast 'shadow blades' %actor.name%
         elseif %level% >= 40
           dg_cast 'shadow dance' %actor.name% 
         else
           kill %actor.name%
         end
         break
       case Assassin
           * Do what I do best
         if %self.eq(19)%
           back %actor.name%
           murd %actor.name%
           murd %actor.name%
         else 
           dust %actor.name%
murd %actor.name%
murd %actor.name%
         end
         break
       case Thief
           * Do what I do best
         if %self.eq(19)%
           back %actor.name%
         else 
           dust %actor.name%
         end
         break    
         case Magic User
           * Do what I do best
         if %level% < 6
           kill %actor.name%
         elseif %level% > 6 && %level% < 25
           dg_cast 'web' %actor.name%
         elseif %level% >=25 && %level% < 30
           dg_cast 'ice storm' 
         elseif %level% >= 30 && %level% < 35
           dg_cast 'sand storm' %actor.name%
         elseif %level% >= 35 && %level% < 40
           dg_cast 'lightning bolt' %actor.name%
         elseif %level% >=40 && %level% < 45
           dg_cast 'meteor swarm'
         elseif %level% >= 45 && %level% < 50
           dg_cast 'fireball' %actor.name%
         else
           dg_cast 'doom bolt' %actor.name%
         end
         break
       default
           * Do what I do best
         kill %actor.name%
       break
  else 
   halt
   end
~
#61
Aggro Look~
0 e 0
looks at you~
say %actor.name%, I don't like people looking at me.
kill %actor.name%
~
#62
Object to Mob transforms~
1 c 100
l~
* This trigger will transform an object into a mob
* of the same vnum (ie obj 1217 becomes mob 1217
* when it is looked at
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
#63
Clan guard superaggro~
0 h 100
~
* Super Aggro script that detects mob class and performs
* accordingly
eval room %self.room%
eval class %self.class%
eval level %self.level%
eval align %self.align%
eval victim %actor%
if %victim.clan% == %room.clan%
  %send% %self% %victim.name% is a clannie
  halt
end 
if %victim.is_npc% != 0 || %victim.level% > 50
  %send% %self% %victim.name% is an immortal, or NPC
   halt
else
  %send% %self% %victim.name% is a valid target
  switch %class%
    case Warrior
        * Do what I do best
        if %self.eq(19)%
          bash %actor.name%
        else
          kill %actor.name%
          kick %actor.name%
        end
      break
    case Ranger
        * Do what I do best
      trip %actor.name%
      break
    case Shou-Lin
        * Do what I do best
      if %level% < 12
        kill %actor.name%
      elseif %level% >= 12 && %level% < 18
        sweep %actor.name%
      elseif %level% >= 18 && %level% < 23
        blind %actor.name%
      elseif %level% >= 23 && %level% < 33
        knock %actor.name%
      else
        fist %actor.name%
      end
      break
    case Cleric
        * Do what I do best
      if %level% >= 43 && %align% < -250
        dg_cast 'unholy word'
      elseif %level% >= 43 && %align% > 250
        dg_cast 'holy word'
      else
        dg_cast 'earthquake'
      end
      break
    case Solamnic Knight
        * Do what I do best
      if %level% < 9
        kill %actor.name%
      elseif %level% >= 9 && %level% < 44 && %self.eq(19)%
        bash %actor.name%
      elseif %level% > 44 && %actor.align% < -250
        dg_cast 'holy word'
      else
        kick %actor.name%
      end
      break
    case Death Knight
        * Do what I do best
      if %level% < 9
        kill %actor.name%
      elseif %level% >= 9 && %level% < 30 && %self.eq(19)%
        bash %actor.name%
      elseif %level% >= 30 && $level% < 45
        dg_cast 'pestilence'
      elseif %level% >= 45 && %actor.align% > 250
        dg_cast 'unholy word'
      else
        kick %actor.name%
      end
      break
    case Shadowdancer
        * Do what I do best
      if %level% < 10 && %self.eq(19)%
        backstab %actor.name%
      elseif %level% >=10 && %level% < 40
        dg_cast 'shadow blades' %actor.name%
      elseif %level% >= 40
        dg_cast 'shadow dance' %actor.name%
      else
        kill %actor.name%
      end
      break
    case Assassin
        * Do what I do best
      if %self.eq(19)%
        back %actor.name%
      else
        dust %actor.name%
      end
      break
    case Thief
        * Do what I do best
      if %self.eq(19)%
        back %actor.name%
      else
        dust %actor.name%
      end
      break
    case Magic User
        * Do what I do best
      if %level% < 6
        kill %actor.name%
      elseif %level% > 6 && %level% < 25
        dg_cast 'web' %actor.name%
      elseif %level% >=25 && %level% < 30
        dg_cast 'ice storm'
      elseif %level% >= 30 && %level% < 35
        dg_cast 'sand storm' %actor.name%
      elseif %level% >= 35 && %level% < 40
        dg_cast 'lightning bolt' %actor.name%
      elseif %level% >=40 && %level% < 45
        dg_cast 'meteor swarm'
      elseif %level% >= 45 && %level% < 50
        dg_cast 'fireball' %actor.name%
      else
        dg_cast 'doom bolt' %actor.name%
      end
      break
    default
        * Do what I do best
      kill %actor.name%
    break
else
halt
end
~
#65
Mage SuperAggro (Sandstorm)~
0 h 100
~
if %actor.is_npc% != 0 || %actor.level% > 50
  halt
else
  dg_cast 'sandstorm' %actor.name%
end
~
#66
Warrior SuperAggro (Bash)~
0 h 100
~
if %actor.is_npc% != 0 || %actor.level% > 50
  halt
else
  bash %actor.name%
end
~
#67
Rogue SuperAggro (backstab)~
0 h 100
~
if %actor.is_npc% != 0 || %actor.level% > 50
  halt
else
  back %actor.name%
end
~
#68
Mobgenerator (Demon Gate)~
0 b 40
~
eval mob %random.10%
if %mob% == 1
 %echo% &08The surface of the portal distorts as a Demon pushes it's way through!&00
 %load% mob 4700
elseif %mob% == 2
 %echo% &08The surface of the portal distorts as a Demon pushes it's way through!&00
 %load% mob 20638
elseif %mob% == 3
 %echo% &08The surface of the portal distorts as a Demon pushes it's way through!&00
 %load% mob 1288
elseif %mob% == 4
 %echo% &08The surface of the portal distorts as a Demon pushes it's way through!&00
 %load% mob 20703
elseif %mob% == 5
 %echo% &08The surface of the portal distorts as a Demon pushes it's way through!&00
 %load% mob 21517
elseif %mob% == 6
 %echo% &08The surface of the portal distorts as a Demon pushes it's way through!&00
 %load% mob 19037
elseif %mob% == 7
 %echo% &08The surface of the portal distorts as a Demon pushes it's way through!&00
 %load% mob 20726
elseif %mob% == 8
 %echo% &08The surface of the portal distorts as a Demon pushes it's way through!&00
 %load% mob 15909
else
 %echo% &10A wave of heat washes over you, and the surface of the portal flexes.&00
   end
~
#69
Mob Generator (Fissure)~
1 b 50
~
eval mob %random.10%
if %mob% == 1
  %echo% &08A sulfurous cloud rises from the fissure, revealing a Demon!&00
  %load% mob 20638
elseif %mob% == 2
  %echo% &08A sulfurous cloud rises from the fissure, revealing a Demon!&00
  %load% mob 20703
elseif %mob% == 3
  %echo% &08A sulfurous cloud rises from the fissure, revealing a Demon!&00
  %load% mob 20726
elseif %mob% == 4
  %echo% &08A sulfurous cloud rises from the fissure, revealing a Demon!&00
  %load% mob 15909
else
  %echo% &07A cloud of steam rises from the fissure, reeking of brimstone.&00
end
~
#70
Singing weapons :)~
1 b 10
~
eval phrase %random.38%
   if %phrase% == 1
     %echo% %self.shortdesc% sings, 'Why can't I get, just one kiss...  oh yeah.. I'm a %self.shortdesc%...'
   elseif %phrase% == 2
     %echo% %self.shortdesc% sings, 'Straaaaaangers in the night'
   elseif %phrase% == 3
     %echo% %self.shortdesc% says, 'Okay, I'm bored now, get on with the killing!'
   elseif %phrase% == 4
     %echo% %self.shortdesc% screams "BANZAI!!!!" and brandishes itself for battle.
   elseif %phrase% == 5
     %echo% %self.shortdesc% says, "Come On! Lets kill something! Kill! Kill! Kill!!!'
   elseif %phrase% == 6
     %echo% %self.shortdesc% says, "You know, I think I'll retire and become a letter opener.. It would be more exciting than hanging out with you!'
   elseif %phrase% == 7
     %echo% %self.shortdesc% says, 'Hey, when you going to hook me up with that cute dagger we saw the other day?'
   elseif %phrase% == 8
     %echo% %self.shortdesc% says, 'I would love to go out with you, but I want to spend more quality time with my sheath...'
   elseif %phrase% == 9
     %echo% %self.shortdesc% says, 'Many people would rather die than think... Me?  I'd rather do the killing...'
   elseif %phrase% == 10
     %echo% %self.shortdesc% says, 'I think we should relax, and think about world peace.....  Oh who am i kidding, lets go spill some blood!'
   elseif %phrase% == 11
     %echo% %self.shortdesc% says, 'My blade is bigger then yours!'
   elseif %phrase% == 12
     %echo% %self.shortdesc% whispers to you, 'Hey, is that a mace in your pocket, or are you just happy to see me?'
   elseif %phrase% == 13
     %echo% %self.shortdesc% says, 'Hey! Back off buddy, or I'm gonna be looking at your lower intestine from the inside!'
   elseif %phrase% == 14
     %echo% %self.shortdesc% sings, 'Somewhere over the rainbow...'
   elseif %phrase% == 15
     %echo% %self.shortdesc% says, 'Take my Knife!  Please!'
   elseif %phrase% == 16
     %echo% %self.shortdesc% says, 'You mind trying not to hit bone so often? I keep getting nicks in my finish!'
   elseif %phrase% == 17
     %echo% %self.shortdesc% sings, 'Somewhere over the rainbow...'
   elseif %phrase% == 18
     %echo% %self.shortdesc% says, 'So these two guys walk into a bar...'
   elseif %phrase% == 19
     %echo% %self.shortdesc% hums ominously.
   elseif %phrase% == 20
     %echo% %self.shortdesc% says, 'You're not even worth flicking a booge at...'
   elseif %phrase% == 21
     %echo% %self.shortdesc% says, 'Romani quidem artem amatoriam invenerunt. '
   elseif %phrase% == 22
     %echo% %self.shortdesc% says, 'If Caesar were alive, you'd be chained to an oar. '
   elseif %phrase% == 23
     %echo% %self.shortdesc% says, 'In the good old days, children like you were left to perish on windswept crags.'
   elseif %phrase% == 24
     %echo% %self.shortdesc% sings, 'Du hast mich gefragt, und ich hab nichts gesagt.'
   elseif %phrase% == 25
     %echo% %self.shortdesc% says, 'That's not a knife!'
   elseif %phrase% == 26
     %echo% %self.shortdesc% says, 'It's a trick!  Get an axe.'
   elseif %phrase% == 27
     %echo% %self.shortdesc% says, 'This is the Voice of Doom calling. Your days are numbered, to the seventh son of the seventh son!'
   elseif %phrase% == 28
     %echo% %self.shortdesc% says, 'The complete lack of humility for nature that's being displayed here is staggering.'
   elseif %phrase% == 29
     %echo% %self.shortdesc% says, 'If I give you another face lift you are going to be able to blink your lips! '
   elseif %phrase% == 30
     %echo% %self.shortdesc% says, 'If the MUD community is a family, think of me as the uncle no one talks about.'
   elseif %phrase% == 31
     %echo% %self.shortdesc% says, 'Are you with me or do you need me to draw it in crayon like usual?'
   elseif %phrase% == 32
     %echo% %self.shortdesc% says, 'Man, That guy would go home with a gardening tool if it showed interest'
   elseif %phrase% == 33
     %echo% %self.shortdesc% says, 'God says he can get me out of this mess, but he's pretty sure you're screwed'
   elseif %phrase% == 34
     %echo% %self.shortdesc% says, 'Throw me a frickin' bone here! I'm the boss! Need the info!'
   elseif %phrase% == 35
     %echo% %self.shortdesc% says, 'Get away from me, you lazy-eyed psycho!'
   elseif %phrase% == 36
     %echo% %self.shortdesc% says, 'You're not real bright are you...  I like that in a man.'
   elseif %phrase% == 37
     %echo% %self.shortdesc% says, 'You want to be worshiped?  Go to India and Moo.'
   else
     %echo% %self.shortdesc%  says, ' WHOA MAMA!  Did you see the Blood grooves on that one?!'
   end
~
#71
The Dice That Work~
1 h 100
~
* better dice
eval DieOne %random.6%
eval Dietwo %random.6%
%echoaround% %actor% %actor.name% drops %self.shortdesc%, which rattle around on the ground.
%send% %actor% You drop %self.shortdesc% which rattle around on the ground.
%echo% The first die lands on %DieOne% and the second on %DieTwo%.
~
#95
Random Trap~
1 g 100
~
eval trapnumber %random.4%
if %trapnumber% == 1
  %echoaround% %actor% Perhaps %actor.name% shouldn't have messed with %self.shortdesc%.
  %send% %actor% Uh oh... You shouldn't have messed with that.
  dg_cast 'chain lightning'
  return 0
  %purge% %self%
elseif %trapnumber% == 2
  %echoaround% %actor% Perhaps %actor.name% shouldn't have messed with %self.shortdesc%.
  %send% %actor% Uh oh... You shouldn't have messed with that.
  dg_cast 'teleport' %actor.name%
  return 0
  %purge% %self%
elseif %trapnumber% == 3
  %echoaround% %actor% Perhaps %actor.name% shouldn't have messed with %self.shortdesc%.
  %send% %actor% Uh oh... You shouldn't have messed with that.
  %echo% &08%self.shortdesc% explodes with a blinding flash!&00
  %echo% &08A wave of heat washes over you!&00
  %damage% %actor% 100
  return 0
  %purge% %self%
else 
  %echo% You hear a loud click but nothing happens.
  return 0 
end
~
#96
Electrical Trap~
1 g 100
~
%echoaround% %actor% Perhaps %actor.name% shouldn't have messed with %self.shortdesc%.
%send% %actor% Uh oh... You shouldn't have messed with that.
dg_cast 'chain lightning'
return 0
%purge% %self%
~
#97
Teleport Trap~
1 g 100
~
%echoaround% %actor% Perhaps %actor.name% shouldn't have messed with %self.shortdesc%.
%send% %actor% Uh oh... You shouldn't have messed with that.
dg_cast 'teleport' %actor.name%
return 0
%purge% %self%
~
#98
Exploding Trap (100)~
1 g 100
~
%echoaround% %actor% Perhaps %actor.name% shouldn't have messed with %self.shortdesc%.
%send% %actor% Uh oh... You shouldn't have messed with that.
%echo% &08%self.shortdesc% explodes with a blinding flash!&00
%echo% &08A wave of heat washes over you!&00
%damage% %actor% 100
return 0
%purge% %self%
~
#99
Exploding Trap (200)~
1 g 100
~
%echoaround% %actor% Perhaps %actor.name% shouldn't have messed with %self.shortdesc%.
%send% %actor% Uh oh... You shouldn't have messed with that.
%echo% &08%self.shortdesc% explodes with a blinding flash!&00
%echo% &08A wave of heat washes over you!&00
%damage% %actor% 200
return 0
%purge% %self%
~
$~
