#3101
Pickpocket Chatter --> attached m3152~
0 b 25
~
eval phrase %random.3%
  if %phrase% == 1
   say When you are ready to learn a new skill, Na'Rooth will help you '&09practice&00'.
  elseif %phrase% == 2
   say I have many items that can be bought in my shop. Simply use the command '&09list&00' to check my wares.
  else
   say Looking for a job around town? I can help! Just use the command '&09quest offer&00'.
end
~
#3102
Na'Rooth Chatter --> attached m3022~
0 g 60
~
if %actor.is_npc% != 0 || %actor.level% > 50
  halt
else
  wait 2 sec
  say Welcome to the Midgaard Thief Guild %actor.name%.
  wait 3 sec
  say When you are ready, you may practice new skills with me.
end
~
$~
