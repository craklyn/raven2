#38401
Arrow Defense SK Guild --> R18110~
2 b 100
~
if %self.people%
   eval victim %random.char%
if %victim.level% > 50 || %victim.class% == Solamnic Knight || %victim.is_npc% != 0
   halt
   end
%echo% a Solamnic Archer shouts, 'Please move away from the keep, this is your first and last warning!'
wait 3 s
%echo% A volley of arrows blackens the sky above Solgaard Keep.
   %damage% %victim% %random.100%
%echoaround% %victim% Several arrows from the towers above hit %victim.name%'s chest.
   %send% %victim% You take several arrows in the chest.
else
   halt
~
#38402
SK Guardian attack --> m38405~
0 h 100
~
if %actor.is_npc% != 0 || %actor.level% > 50
   halt
  elseif %actor.class% == Solamnic Knight
wait 1 s
   say Greetings %actor.name%! I hope you have been spreading the gift of light in your adventures.
else
   wait 1 s
   say You're not part of the Solamnic Knight Order! I shall smite thee evil vermin!
   kill %actor.name%
end
~
#38403
SK Wandering Guard attack --> m38405~
0 i 100
~
eval inroom %self.room%
eval person %inroom.people%
wait 1 sec
while %person%
if %player.vnum% == -1 && %person.class% != cleric
say You dare infiltrate the sacred Solamnic Knight Guild?!? Prepare for my wrath!
mkill %person.name%
end
eval person %person.next_in_room%
done
~
#38404
Chapel Heal Bonus -->R38412~
2 b 5
~
%echo% A beam of &14light&00 pierces the chapel's arched window.
~
$~
