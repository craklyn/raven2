#55101
FrogQueen~
0 d 100
"what do i need frog queen"~
wait 10
say I know you need a drop of sun to light your way.
wait 10
%load% obj 55105
wait 10
give all %actor.name%
wait 10
mecho The Frog Queen jumps in to the water and dissapears.
mgoto 10501
wait 720
mgoto 55175
~
#55102
frogqueen2~
0 d 100
"what do i need frog queen"~
wait 3
say I know you need a drop of sun to light your way.
wait 3
%load% obj 55105
wait 3
give all %actor.name%
wait 300
%load% mob 55103
mgoto 10501
~
#55103
ThankTheQueen~
2 d 100
"thank you frog queen" "thank you queen" "thanks frog queen" "thanks queen"~
wait 5
%echo% The Frog Queen waves and bids you a safe trip.
~
#55111
another easter trigger~
0 j 100
~
if (%object.vnum% != 55111)
say I'm sorry but that's not what im after :(
return 0
else
say Thanks! You've made my day. Here is your reward!
dg_cast 'fast learn' %actor.name%
end
~
$~
