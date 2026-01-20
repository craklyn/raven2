#43401
Talker for mob43415~
0 g 50
~
say Beware the Tower, for your unending torment lies there!
~
#43402
Angel talk trigger~
0 g 100
~
emote studies you intently.
wait 2
say So...  You have come to be reborn?
wait 2
emote makes a quaint magical gesture and creates a demon!
load mob 43417
end
~
#43425
Sand storm on entry~
0 g 100
~
if %actor.is_npc% == 0 && %actor.name% && %actor.level% < 51
dg_cast 'sand storm' %actor.name%
end
~
#43426
Clan teleport defense~
2 g 100
~
   if %actor.clan% == %room.clan%
     %echo% stage 1
     halt
   if %actor.is_npc% != 0 || %actor.level% > 50
     %echo% stage 2
     halt
   else
          %echo% stage 3
     dg_cast 'teleport' %actor%
       end
       break
   else
     %echo% stage 4
   halt
   end
~
#43427
Rogue Secret Spellup Trigger~
2 c 100
gimme~
dg_cast 'sanct' %actor%
dg_cast 'shield' %actor%
dg_cast 'stoneskin' %actor%
dg_cast 'blur' %actor%
dg_cast 'pulse heal' %actor%
dg_cast 'pulse gain' %actor%
dg_cast 'shadow sphere' %actor%
dg_cast 'cleanse' %actor%
dg_cast 'fly' %actor%
dg_cast 'regenerate' %actor%
dg_cast 'shadow vision' %actor%
dg_cast 'true sight' %actor%
dg_cast 'haste' %actor%
dg_cast 'dance of shadow' %actor%
dg_cast 'flame blade' %actor%
dg_cast 'bless' %actor%
dg_cast 'shadow walk' %actor%
dg_cast 'airsphere' %actor%
dg_cast 'fast learn' %actor%
~
#43428
Hassle player trigger~
1 c 100
look~
%force% %actor% seduce %actor.name%
~
#43430
Tom Turkey Thanksgiving trigger~
0 f 80
~
eval chance %random. 3%
switch %chance%
  case 1
    %load% obj 15455
  break
  case 2
    %load% obj 24350
  break
  case 3
    %load% obj 24349
  break
  default
    %echo% error in Thanksgiving Quest. Tell Fenrir asap.
  Done
  eval master %actor% 
  eval more 1
  while %more%
    if %master.master% && (%master.is_npc% || %master.is_grouped%)
      eval master %master.master%
    else
      eval more 0
    end
  done
  ; reward the master
  if %master.is_npc% == 0 && %master.room% == %actor.room% && %master.is_grouped%
    %send% %master% You have earned 1 qp!
    nop %master.qp(1)%
  end
~
#43431
Tom Turkey's Thanksgiving flee trigger~
0 k 25
~
%echo% Tom Turkey gobbles furiously and flies from the room.
flee
~
#43432
Teleport to Xerxes trap~
1 g 100
~
%echoaround% %actor% Perhaps %actor.name% shouldn't have messed with %self.shortdesc%.
%send% %actor% Uh oh... You shouldn't have messed with that.
%teleport% %actor.name% 14786
%force% %actor% look
%purge% %self%
~
#43435
Stupid rock trigger 2~
1 b 2
~
eval phrase %random.7%
      if %phrase% == 1
%echo% %self.shortdesc% asks, '&12What are you looking at rock for brains?&00'
      elseif %phrase% == 2
        %echo% %self.shortdesc% looks at you and says, '&14And people think I'm dense?!?&00'
      elseif %phrase% == 3
%echo% %self.shortdesc% shouts, '&09Let's get this party started!&00'
      elseif %phrase% == 4
        %echo% %self.shortdesc% says '&12Whoever said moss doesn't grow on a rolling stone never looked at the fungus on your feet!&00' 
      elseif %phrase% == 5
%echo% %self.shortdesc% looks at you carefully and says '&14Look at it this way. From looking at you, there's no way to go but up&00.'
elseif %phrase% == 6
%echo% %self.shortdesc% says, '&10If I was as ugly as you, I would hide behind a mask too.&00'
else
%echo% %self.shortdesc% says, '&09Rock and Roll forever!&00'
      end
~
#43436
Transportation test trigger~
2 d 100
"take me away"~
%echo% It works.
end
~
#43437
Aggro Player Name trigger~
0 g 100
~
if %actor.name% == dvorak
say Time to die %actor.name%
kill %actor.name%
end
~
#43440
Christmas Present Trigger~
1 g 100
~
eval trapnumber %random.5%
   if %trapnumber% == 1
     %echoaround% %actor% %actor.name% opens the Christmas present.
     %echoaround% %actor% Santa Claus appears and offers %actor.name% a tall glass of milk and a plate of cookies.
     %send% %actor% Santa Claus appears with some milk and cookies. As you eat them, you feel energized!
     dg_cast 'haste' %actor.name%
     dg_cast 'sanctuary' %actor.name%
     return 0
     %purge% %self%
   elseif %trapnumber% == 2
     %echoaround% %actor% %actor.name% opens the Christmas present.
     %echoaround% %actor% Santa Claus appears and offers %actor.name% a tall glass of milk and a plate of cookies.
     %send% %actor% Santa Claus appears with some milk and cookies. As you eat them, you feel energized!
     dg_cast 'pulse gain' %actor.name%
     dg_cast 'pulse heal' %actor.name%
     return 0
     %purge% %self%
   elseif %trapnumber% == 3
     %echoaround% %actor% %actor.name% opens the Christmas present.
     %echoaround% %actor% %actor.name% is weighed on the scales of naughty and nice and ends up in the balance.
     %send% %actor% You think perhaps you should have done a little more good this year.
     dg_cast 'ward' %actor.name%
     dg_cast 'shield' %actor.name%
     dg_cast 'heal' %actor.name%
     dg_cast 'poison' %actor.name%
     return 0
     %purge% %self%
   elseif %trapnumber% == 4
     %echoaround% %actor% %actor.name% opens the Christmas present.
     %echoaround% %actor% Satan Claus appears and starts throwing lumps of coal at %actor.name%. 
     %send% %actor% Satan Claus appears before you and he doesn't look happy with you.
     %send% %actor% With a great HO! HO! HO! Satan Claus releases his fury upon you!
     dg_cast 'black breath' %actor.name%
     dg_cast 'death touch' %actor.name%
     return 0
     %purge% %self%
   elseif %trapnumber% == 5
     %echoaround% %actor% %actor.name% opens the Christmas present.
     %echoaround% %actor% Satan Claus appears and starts throwing lumps of coal at %actor.name%. 
     %send% %actor% Satan Claus appears before you and he doesn't look happy with you.
     %send% %actor% With a great HO! HO! HO! Satan Claus releases his fury upon you!
     dg_cast 'chill touch' %actor.name%
     dg_cast 'disease' %actor.name%
     return 0
     %purge% %self%
   else 
     %echo% There doesn't appear to be anything inside the present. You wonder who took your gift.
     return 0 
   end
~
#43441
Satan Claus' Xmas present trigger~
0 g 20
~
if %actor.is_npc% !=0
halt
else
say &08HO! &12HO! &14HO!&00 Let me see who's been naughty or nice.
%echo% Satan Claus reads over his list and says, 'Let's let the present be a suprise.'
%load% obj 43430
drop present
end
~
#43442
Stupid object trigger~
1 j 100
~
%send% %actor% &02I guess this gives new meaning to the phrase '&03rocks for brains&02.'&00
%echoaround% %actor% &07You hear a rattling sound and look up to see a small rock fall out of %actor.name%'s ear and begin to circle %actor.name%'s head.&00
end
~
#43443
Frosty's reward for help trigger~
0 g 100
~
if %actor.is_npc% != 0
halt
end
%echoaround% %actor% &13Frosty&00 the &14Snowman&00 energetically shakes %actor.name%'s hand and says, 'Please accept this aid in return for the aid your people lent me.'
%send% %actor% &13Frosty&00 the &14Snowman&00 energetically shakes your hand and says, 'Please accept this aid in return for the aid your people lent me.'
eval reward %random.4%
if %reward% == 1
  %send% %actor% &13Frosty&00 the &14Snowmans&00 magic &24top hat&00 glows with a &21white&00 light.
    dg_cast 'sanctuary' %actor.name%
elseif %reward% == 2
  %send% %actor% &13Frosty&00 the &14Snowmans&00 magic &24top hat&00 glows with a &15red&00 light.
    dg_cast 'haste' %actor.name% 
elseif %reward% == 3
  %send% %actor% &13Frosty&00 the &14Snowmans&00 magic &24top hat&00 glows with a &16green&00 light.
    dg_cast 'revive' %actor.name%
    dg_cast 'revive' %actor.name%
    dg_cast 'revive' %actor.name%
else
  %send% %actor% &13Frosty&00 the &14Snowmans&00 magic &24top hat&00 glows with a &18blue&00 light.
    dg_cast 'stoneskin' %actor.name%
    dg_cast 'shield' %actor.name%
    dg_cast 'true sight' %actor.name%
    dg_cast 'fly' %actor.name%
    dg_cast 'regen' %actor.name%
end
~
#43444
Aggro Human Trigger for mobs~
0 g 100
~
if %actor.race% == human
say die human scum
kill %actor.name% 
else end
end
~
#43445
Leprechaun's flee trigger~
0 k 12
~
shout You can't have me gold!
flee
end
~
$~
