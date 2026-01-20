#1601
easterbunnytrigger~
0 g 1
~
%load% obj 43452
put egg basket
%echo% The Easter Bunny grabs an Easter egg from its basket.
get egg basket
drop egg
~
#1602
easterbunny trigger 2~
0 an 50
~
mload obj 43452     
put egg basket
~
#1650
Cupid valentines day~
0 g 50
~
wait 50
say Happy &08Valentines&00 Day!!
wait 100
drop candy
end
~
#1691
halloween candy~
0 d 0
trick or treat~
if (%actor.is_npc% != 0 || %actor.level% > 50)
  halt
elseif %speech% != trick or treat
  halt
end
eval quip %random.3%
if %quip% == 1
  say My, you're dressed awfully scary this year!
elseif %quip% == 2
  say What are you this year, little guy?
elseif %quip% == 3
  say What big teeth you have!
end 
eval item 1690 + %random.4%
eval trickortreat %random.4%
wait %random.2% s
if %trickortreat% == 1
  say OH I HAVE A TRICK FOR YOU!!
  eval trick %random.8%
  switch %trick%
    case 1
      dg_cast 'blind' %actor.name%
      dg_cast 'blind' %actor.name%
      dg_cast 'blind' %actor.name%
      dg_cast 'blind' %actor.name%
      dg_cast 'blind' %actor.name%
      dg_cast 'blind' %actor.name%
    break
    case 2
      dg_cast 'poison' %actor.name%
      dg_cast 'poison' %actor.name%
      dg_cast 'poison' %actor.name%
      dg_cast 'poison' %actor.name%
      dg_cast 'poison' %actor.name%
      dg_cast 'poison' %actor.name%
    break
    case 3
      dg_cast 'debilitate' %actor.name%
      dg_cast 'debilitate' %actor.name%
      dg_cast 'debilitate' %actor.name%
      dg_cast 'debilitate' %actor.name%
      dg_cast 'debilitate' %actor.name%
      dg_cast 'debilitate' %actor.name%
    break
    case 4
      dg_cast 'energy drain' %actor.name%
      dg_cast 'energy drain' %actor.name%
      dg_cast 'energy drain' %actor.name%
      dg_cast 'energy drain' %actor.name%
      dg_cast 'energy drain' %actor.name%
      dg_cast 'energy drain' %actor.name%
    break
    case 5
      dg_cast 'feeblemind' %actor.name%
      dg_cast 'feeblemind' %actor.name%
      dg_cast 'feeblemind' %actor.name%
      dg_cast 'feeblemind' %actor.name%
      dg_cast 'feeblemind' %actor.name%
      dg_cast 'feeblemind' %actor.name%
    break
    case 6
      dg_cast 'curse' %actor.name%
      dg_cast 'curse' %actor.name%
      dg_cast 'curse' %actor.name%
      dg_cast 'curse' %actor.name%
      dg_cast 'curse' %actor.name%
      dg_cast 'curse' %actor.name%
    break
    case 7
      dg_cast 'silence' %actor.name%
      dg_cast 'silence' %actor.name%
      dg_cast 'silence' %actor.name%
      dg_cast 'silence' %actor.name%
      dg_cast 'silence' %actor.name%
      dg_cast 'silence' %actor.name%
    break
    case 8
      dg_cast 'dance of dreams' %actor.name%
      dg_cast 'dance of dreams' %actor.name%
      dg_cast 'dance of dreams' %actor.name%
      dg_cast 'dance of dreams' %actor.name%
      dg_cast 'dance of dreams' %actor.name%
      dg_cast 'dance of dreams' %actor.name%
    break
    default
      %echo% error in Jack's trick, tell an imm
    done
  else
    %load% obj %item%
    give candy %actor.name%
  end
~
$~
