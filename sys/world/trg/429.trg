#42900
scarecrow --> Chisha Fields(room)~
2 g 100
~
wait 1 s
%echo% &00&03A flock of crows resting upon a beaten scarecrow scatter as they hear movement.&00
~
#42901
fountain~
2 g 20
~
wait 1 s
%echo% &14The soft soothing sound of water echos here, emanating from the statue in the center of the fountain.&00
~
#42902
bar 1~
2 g 100
~
wait 1 s
%echo% The bartender clears a few empty glasses on the bar and refills the snack dish for the next customer.
~
#42903
bar 2~
2 b 5
~
%echo% A waitress walks by carrying a bunch of drinks on a tray headed towards one of the seated tables.
~
#42904
bar 3~
2 b 5
~
%echo% A waiter buses an empty table, removing the garbage left behind by the previous customer.&00
~
#42905
bar 4~
2 b 5
~
%echo% A customer sitting at the bar finishes his drink, leaves a meager tip for the bartender, and exits the tavern.
~
#42906
bar 5~
2 b 5
~
%echo% The bartender refills an empty drink for a customer and gives the bar a wipe down with his towel.
~
#42907
Quest~
0 g 100
~
%echo% The bartender can be overheard speaking to one of the patrons at the bar.
wait 1 s
say I once made a special drink for King Xerxes himself!
wait 1 s
sigh
say If only I could gather those ingredients again...
~
#42908
Yun~
0 k 5
~
%echo% Yun takes a step back and begins to concentrate deeply, focusing all of her energy into destroying %actor.name%.
dg_cast 'doom bolt' %actor%
~
#42909
Yun teleport~
0 k 5
~
%echo% Yun's eyes turn a pure white as she raises her hand towards %actor.name%, 'I banish you from these sister three!'
%teleport% %actor% 42963
%echo% %actor.name% vanishes suddenly.
~
#42910
wand alma kinan~
1 j 100
~
wait 1 s
%echo% %actor.name% is enveloped in a bright white light as the power of the Alma Kinan surges through them.
~
#42911
Transfer to Three Sisters~
0 d 0
Alma Kinan~
peer %actor%
wait 1 s
say So you wish to learn more about the Alma Kinan?
grin
wait 1 s
think
wait 1 s
say Very well, then, I wish you good luck!
%echo% Chris Lightfellow waves her right hand towards %actor.name% and a bright light envelopes them.
%teleport% %actor% 42998
end
~
#42912
Yumi Teleports --> m(42911)~
0 k 10
~
eval target % random.char %
if % target.is_npc % || % target.level % >50
        halt
else
    if % target %
        say I call upon the goddess Alma Kinan to send you from whence you came!
        % teleport % % target % 18001
        force % target % look
        mat 18001 % echo % % target.name % phases out of existance.
        end
~
#42913
Yuiri~
0 k 10
~
%echo% Yuiri focuses her ki and then unleashes a very powerful punch towards %actor.name%.
fist
~
#42914
Yumi sac~
2 d 100
legna~
%load% m 42908
~
#42915
Yun sac~
0 f 100
~
%load% o 42925
~
#42916
Yun silence~
0 e 0
stares at you and utters~
if (%actor.name% == yumi)
wait 1 s
thank yumi
else
wait 1 s
%echo% Yun turns towards %actor.name% and raises her hand towards them sending forth a beam of destruction.
dg_cast 'doom bolt' %actor%
~
#42917
yumi cast~
0 e 0
stares at you and utters the words~
wait 1 s
%echo% Yumi calls upon her goddess and her eyes turn a blood red as the power of Alma Kinan begins to enter her body.
dg_cast 'flame strike' %actor%
~
#42918
wheat falling~
1 b 1
~
%echo% An ear of wheat falls from the stalk onto the dirt below.
%load% o 42903
~
#42919
maize falling~
1 b 1
~
%echo% An ear of maize falls from the stalk onto the dirt below.
%load% o 42901
~
#42920
barley falling~
1 b 1
~
%echo% An ear of barley falls from the stalk onto the dirt below.
%load% o 42905
~
#42921
ear~
1 b 15
~
%echo% The ear of grain rots fully and disintegrates into nothingness.
%purge% %self%
~
#42922
Geddoe Intro Speech -->m42917~
0 g 100
~
wait 1 s
%echo% Geddoe looks up as %actor.name% enters his bedroom.
wait 1 s
%echo% Geddoe raises his eyebrow in question and states, 'How may I help you?'
~
#42923
Geddoe Story Speech -->m42917~
0 d 1
flame champion~
%echo% Geddoe sighs and says, 'I hope you have a second, because what you're asking requires great detail.'
wait 1 s
say Long ago, there was a war between the powerful oppressive force in Harmonia and the Grassland Clans. 
wait 1 s
say The Grassland Clans were a group of villages forged together in their pursuit from freedom from Harmonia. However, they were crushed quite easily by the Harmonian forces until the arrival of the Flame Champion. 
wait 1 s
say His real name not known, the Flame Champion was the heroic figure who liberated the Grasslands from Harmonian oppression with the help of the True Fire Rune and his Fire Bringer.
wait 1 s
say He began attacking Harmonian convoys with his small band of men, and as his organization grew, he recruited more people. 
wait 1 s
say However, to free a large amount of Grassland prisoners being held by Harmonia and to enter a time of peace, he traded his freedom, and was incarcerated in Harmonia.
wait 1 s
say He was murdered while imprisoned by a woman named Sarah, who received the True Fire Rune upon her evil act. Wielding this extremely powerful rune, she has plagued the Grassland people ever since. 
wait 1 s
say The Chishan Village was formed to create a unified force made up of the remaining Grassland clans that had been decimated by the second Flame Champion and her Fire Bringer. 
wait 1 s
say Bringing all of the clans together was essential in ever being able to have a joint effort in defeating her. 
wait 1 s
%echo% Geddoe looks towards the window.
wait 1 s
say I joined the council in the beginning and was later chosen to lead this village.
~
#42924
Sana~
0 g 100
~
%echo% Sana looks up from her scroll as %actor.name% enters the bedroom but quickly returns to her work.
~
#42925
Nadir~
0 b 10
~
%echo% Nadir performs his latest masterpiece depicting the war against the Second Flame Champion for anyone interested in watching.
~
#42926
Yuber~
0 g 100
~
wait 1 s
%echo% &07The shadows within this area of the city begin to swirl about strangely.&00
wait 1 s
%echo% A barely audible voice whispers to you, 'I have a job for you to do.'
~
#42927
Orb~
1 c 2
examine~
%echo% &01The orb bursts into a dark red light which consumes it immediately.&00
%damage% %actor% 200
A whirling wind of flames begans to churn within the small side street and eventually climax into a burst of bright flames.
%load% m 42948
%purge% self
~
#42928
load Flame Champion~
0 n 100
~
%load% o 42930
wear tiara
%load% o 42929
wear sand
mgoto 42948
wait 1 s
shout I've been waiting for this day to arrive for so long. Not so tough without Geddoe and Chris here to protect you, are you?
wait 1 s
shout Prepare to feel my wrath!
%load% m 42949
%load% m 42949
%load% m 42949
%load% m 42949
%load% m 42949
%load% m 42949
%load% m 42949
%load% m 42949
%load% m 42949
%load% m 42949
wait 3 s
%echo% Hugo shouts, 'Loyal citizens of Chisha! Quickly! You must take shelter immediately!'
~
#42929
load Firebringer~
0 n 100
~
wait 1 s
shout Prepare to die by the sword of the Firebringer, infidels!
wait 2 s
w
~
#42930
Flame Champion~
0 g 100
~
mkill %actor.name%
shout Prepare to meet your doom, %actor.name%!
~
#42931
Flame Champion special attack~
0 k 10
~
%echo% &01The Flame Champion&00 eyes turn white as she points towards the west.&00
%teleport% %actor% 42945
%echo% %actor.name% was sent flying west by an unseen force.
~
#42932
Firebringer death~
0 f 100
~
%echo% A member of the &08Firebringer&00 screams out for his liege to save him, but receives no mercy.
~
#42933
Knight~
0 f 100
~
%load% m 42947
~
#42934
Luc Load~
0 f 100
~
%echo% &01Sarah, the Flame Champion&00 screams out, 'Luc! How can you allow them to do this to me?'
%load% m 42947
%echo% &06The ashes begin to swirl around rapidly as a dark figure appears, grabs onto %actor.name% and disappears in an instant.&00
%teleport% %actor% 42940
~
#42940
BarCommotion -->r42960~
2 b 100
~
eval song %random.5%
  switch %drunkard%
    case 1
      %echo% A waitress walks by carrying a bunch of drinks on a tray headed towards one of the seated tables.
       break
      case 2
       %echo% A waiter buses an empty table, removing the garbage left behind by the previous customer.
       break
      case 3
       %echo% The bartender clears a few empty glasses on the bar and refills the snack dish for the next customer.
       break
      case 4
       %echo% The bartender refills an empty drink for a customer and gives the bar a wipe down with his towel.
       break
      case 5
       %echo% A customer sitting at the bar finishes his drink, leaves a meager tip for the bartender, and exits the tavern.
       break
    done
~
#42941
Corpse~
0 e 0
drops the corpse~
%echo% &01Yun's eyes turn a deep red in anger as she stares at %actor.name%.&00
sa How dare you try to use the ancient necromancer magic against me!
dg_cast 'doom bolt' %actor%
wait 1 s
destroy corpse
~
$~
