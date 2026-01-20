#23097
Holy Hand Grenade Trigger~
1 h 100
~
* This trigger was adapted from a similar one at
* Builderacademy.net by Raven's Craklyn
* a small script to make a bomb go off three seconds after it's dropped.
set room_var %actor.room%
%echo% The room number is %room_var%
* Send a message when the bomb goes off.
wait 3 s
%echo% %self.shortdesc% blasts into smithereens, striking everyone here.
* Target the first char
set target_char %room_var.people%
%echo% The following are probably the culprits:  %target_char%, %target_char.name%, %actor%, or %actor.name%.
* Do stuff
while %target_char%
%echo% Entering the loop.
%echo% Targetting player %target_char.name%
* Set the next target before this one perhaps dies.
set tmp_target %target_char.next_in_room%
* This is where the good/bad things are supposed to happen.
%send% %target_char% The explosion hurts you.
%echoaround% %target_char% %target_char.name% is hurt by the %self.shortdesc%'s explosive force.
%damage% %target_char% 70
* Find next target
set target_char %target_char.next_in_room%
* Loop back
done
~
$~
