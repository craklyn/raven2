#3300
Guard Warning --> Trigger attached to mob 3319~
0 g 100
~
If %direction% == south
wait 3 s
say Beware of the forests north. Many unsuspecting travelers have been ambushed by a group of thieving scoundrels. 
wait 2 s
wave 
end
~
#3301
Greize's offer --> Trigger attached to mob 3320~
0 g 60
~
wait 5
say You look like a tough lad, how would you like a job. I'll make you an offer you can't refuse.
wait 19
say Don't waste my time just give me a "yes" or "no"....
end
~
#3302
No to Greize's offer --> Attached to mob 3320~
0 d 100
no~
wait 2 s
say I see, guess being a killing machine that spits blood and eats beating hearts ain't for everyone. 
wait 1 s
say Maybe you should go check out the "Crystal Chalice Wine Tasters Club"?
wait 1 s
roll
end
~
#3303
Yes to Greize's offer --> Attached to mob 3320~
0 d 100
yes~
wait 1 sec
say Great, here is my problem. Me and my men take care of Midgaard, but nothing seems to stop those damn thieves from the northern forests.
wait 1 sec
say I need someone to go and cause total chaos in their encampment, you see it move, you kill it. 
wait 1 sec
say All you have to do is bring me a scalp for each one of the bastards you kill and I'll give you 800 coins per kill.
wait 1 sec
say Good hunting Sir.
end
~
#3304
test door trigger~
2 c 100
departs~
if %arg% == north
%echoaround% %actor% As %actor.name% departs north you smile.
end
~
$~
