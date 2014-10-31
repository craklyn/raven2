#18002
Big Appetite~
0 j 100
~
context %actor.id%
say I just switched to context %actor.id%
say In this context, got_milk == %got_milk%
say And, got_muffin == %got_muffin%
say The object vnum you're giving me is %object.vnum%
* Was it a blueberry muffin?
if %object.vnum% == 18041
    if %got_muffin% == yes
        say I don't want another one, thanks!
        return 0
        halt
    end
    set got_muffin yes
    global got_muffin
    eat muffin
    say Oooh, delicious!
* Ok, well, was it a milk-like potion?
elsif %object.vnum% == 18049
    if %got_milk% == yes
        say I don't want any more, thanks!
        return 0
        halt
    end
    set got_milk yes
    global got_milk
    quaff milk
    say That hit the spot!
else
    say I don't want your junk!
    return 0
    halt
end 
if %got_muffin% == yes && %got_milk% == yes
    say I feel completely satisfied, thank you very much!
    : gives you a big pile of coins.
    %send% %actor% There were 10000 coins.
    nop %actor.gold(10000)%
end 
~
#18005
gate guards~
0 g 100
~
wait 2 s
open gate
wait 10s
close gate
end
~
#18006
open west gate~
2 d 100
open~
if %actor.vnum% > 0
wait 2 s
%echo% You hear the great chains start to groan and the gate begins to rise.
wdoor 18093 west room 18098
wdoor 18098 east room 18093
wait 10 s
%echo% The huge gate slams back closed.
wdoor 18093 west purg
wdoor 18098 east purg
end
~
#18007
west gate guards~
0 g 100
~
wait 2 s
bow
end
~
#18008
leave west gate~
2 g 100
~
if %direction% == east
wait 2 s
%echo% You hear the great chains start to groan and the gate begins to rise.
wdoor 18093 west room 18098
wdoor 18098 east room 18093
wait 10 s
%echo% The huge gate slams back closed.
wdoor 18093 west purg
wdoor 18098 east purg
end
~
#18009
Ruby Q speech -- > No answer~
0 d 100
no~
wait 3
say Bah, I knew you weren't up to the challenge, go talk to the Mayor if you're interested in quests for the weak.
end
~
#18010
Ruby Q Speech --> Yes answer~
0 d 100
yes what maybe~
wait 3
say Great! Prepare yourself for an adventure like none before. Simply type 'Quest Offer' for more info. 
end
~
#18011
Ruby Q speech --> Ask (greet trigger)~
0 g 50
~
If %direction% == north
wait 3 
say I believe I can make a shield that could disrupt any type of dragon breath, would you be interested in such an item? 
wait 5 s
say Don't waste my time, just give me a simple yes or no.
end
~
#18080
meow~
0 b 2
~
say meow
~
#18081
transform~
0 d 1
rosebud wolfsbane~
eval myvnum %self.vnum%
switch %myvnum%
case 18081
if %speech% == rosebud
%echo% The puppy falls to the ground, writhing in agony.
wait 10
%echo% You hear the sound of bones and cartilage popping as the puppy begins to grow.
wait 30
mtransform -18082
growl
%echo% The Werewolf snarls at %actor.name%
Kill %actor.name%
end
halt
break
case 18082
if %speech% == wolfsbane
%echo% The werewolf rapidly shrinks into a cute little puppy.
wait 10
mtransform -18081
pant
end
halt
break
~
#18082
transback~
0 d 1
wolfsbane~
mtransform -18081
pant
~
#18084
forsaken clan trigger~
2 d 1
ff~
if %actor.clan% != 2
%send% %actor% What!  You must be drunk!  Back to the dump where you belong, Hobo!
%damage% %actor% 100
%teleport% %actor% 3030
halt
end 
%send% %actor% A raging vortex composed of dark energy consumes your body, burning and thrashing your flesh until you dematerialize.
%teleport% %actor% 19304
%force% %actor% look
%echo% %actor.name% phases out of existence.
~
#18085
outlaw clan trigger~
2 d 100
removeme~
if %actor.clan% != 20
   %send% %actor% What!  You must be drunk!  Back to the dump where you belong, Hobo!
   %damage% %actor% 100
   %force% %actor% dismount
   %teleport% %actor% 3030
   halt
   end 
   %send% %actor% A raging vortex composed of dark energy consumes your body, burning and thrashing your flesh until you dematerialize.
   %force% %actor% dismount
   %teleport% %actor% 19336
   %force% %actor% look
   %echo% %actor.name% phases out of existence.
~
#18086
Republic clan teleporter~
2 d 100
chs~
if %actor.clan% != 3
      %send% %actor% What!  You must be drunk!  Back to the dump where you belong, Hobo!
      %damage% %actor% 100
      %force% %actor% dismount
      %teleport% %actor% 3030
      halt
      end 
%send% %actor% A raging vortex composed of dark energy consumes your body, burning and thrashing your flesh until you dematerialize.
      %force% %actor% dismount
%teleport% %actor% 19318
      %force% %actor% look
      %echo% %actor.name% phases out of existence.
~
#18087
aftermath clan trigger~
2 d 100
yar~
if %actor.clan% != 20
   %send% %actor% What!  You must be drunk!  Back to the dump where you belong, Hobo!
   %damage% %actor% 100
   %force% %actor% dismount
   %teleport% %actor% 3030
   halt
   end 
   %send% %actor% A raging vortex composed of dark energy consumes your body, burning and thrashing your flesh until you dematerialize.
   %force% %actor% dismount
   %teleport% %actor% 19336
   %force% %actor% look
   %echo% %actor.name% phases out of existence.
~
#18089
dark lotus clan trigger~
2 d 1
dg~
if %actor.clan% != 15
%send% %actor% What!  You must be drunk!  Go back to &25&08Hell&00 where you belong, fool!
%damage% %actor% 150
%teleport% %actor% 3030
halt
end
%send% %actor% A raging vortex composed of &07dark&00 energy consumes your body, burning and thrashing your flesh until you dematerialize.
%teleport% %actor% 19312
%force% %actor% dismount
%force %actor% look
%echo% %actor.name% phases out of existence.
end
~
#18196
slick willy greet -> 18199~
0 g 100
~
if (%actor.is_npc% != 0 || %actor.level% > 50)
      halt
else
   say Greetings Adventurer, would you care to test your skills with a little wager?
   wait 50 
   say Don't waste my time, just give me a simple yes or no.
end
~
#18197
slick willy answer on greet -> 18199~
0 d 1
yes no maybe what~
if (%speech% == yes)
   say Terrific! How much would you like to wager? 1000? 10,000? 100,000? or are you slightly more daring and want to try for a cool mil, 1,000,000?
elseif (%speech% == maybe)
   say Well, make up your mind ninny!
elseif (%speech% == what)
   say What? Are you hard of hearing? I said would you care to test your skills with a little wager?
elseif (%speech% == no)
   say say Bah, I knew you didn't have the guts.  Come back when you've become a man.
else
  halt
end
~
#18198
slick willy wager -> 18199~
0 d 1
1000 10000 100000 1000000~
context %actor.id%
if (%actor.is_npc% != 0)
    say You're not allowed to play!
    halt
else
if (%speech% == 1000)
           set offer 1000
                       if %actor.gold% < %offer%
                          say What are you trying to pull here? You cann't cover that much!  Come back when you have some money!
                          set wager 1
                          global wager
                       else
                          set wager 1000
                          set payout 2900
                          global wager
                          global payout
                          %force% %actor.name% give %wager% coins %self.name%
                     say Ok, now just guess the position of the bean.  left, right or center.
                       end                          
elseif (%speech% == 10000)
      set offer 10000
                       if %actor.gold% < %offer%
                          say What are you trying to pull here? You cann't cover that much!  Come back when you have some money!
                          set wager 1
                          global wager
                       else
                          set wager 10000
                          set payout 29000
                          global wager
                          global payout
                          %force% %actor.name% give %wager% coins %self.name%
                     say Ok, now just guess the position of the bean.  left, right or center.
                       end
elseif (%speech% == 100000)
              set offer 100000
                       if %actor.gold% < %offer%
                          say What are you trying to pull here? You cann't cover that much!  Come back when you have some money!
                          set wager 1
                          global wager
                       else
                          set wager 100000
                           set payout 290000
                          global wager
                          global payout
                          %force% %actor.name% give %wager% coins %self.name%
                     say Ok, now just guess the position of the bean.  left, right or center.
                       end
elseif (%speech% == 1000000)
              set offer 1000000
                       if %actor.gold% < %offer%
                          say What are you trying to pull here? You cann't cover that much!  Come back when you have some money!
                          set wager 1
                          global wager
                       else
                          set wager 1000000
                          set payout 2900000
                          global wager
                          global payout
                          %force% %actor.name% give %wager% coins %self.name%
                     say Ok, now just guess the position of the bean.  left, right or center.
                       end
else
           set wager 1
           global wager
      say That's not a valid option.  Please begin by making a wager of 1000, 10000, 100000, or 1000000 coins.
end
end
~
#18199
slick willy gamble -> 18199~
0 d 1
left right center~
context %actor.id%
if (%actor.is_npc% != 0)
    say You're not allowed to play!
    halt
else
if (%speech% == left && (%wager% == 1000 || %wager% == 10000 || %wager% == 100000 || %wager% == 1000000))
       set guess 1
                 set location left
elseif (%speech% == center && (%wager% == 1000 || %wager% == 10000 || %wager% == 100000 || %wager% == 1000000))
       set guess 2
                 set location center
elseif (%speech% == right && (%wager% == 1000 || %wager% == 10000 || %wager% == 100000 || %wager% == 1000000))
       set guess 3
                 set location right
        else
       say Either thats not a valid option, or you think I'm a dope.  Please begin by making a wager of 1000, 10000, 100000, or 1000000 coins.
           halt
end
end
eval position %random.3%
if (%position% == 1 && %guess% == 1)
   tell %actor.name% You have wagered %wager% coins on the %location% shell.
   wait 5
   tell %actor.name% You have chosen wisely.
      if %wager% == 1000
         nop %actor.gold(2900)%
         %send% %actor% %self.name% gives you %payout% coins.
         %echoaround% %actor% %self.name% gives %actor.name% a large pile of gold coins.
      elseif %wager% == 10000
         nop %actor.gold(29000)%
         %send% %actor% %self.name% gives you %payout% coins.
         %echoaround% %actor% %self.name% gives %actor.name% a large pile of gold coins.
      elseif %wager% == 100000
         nop %actor.gold(290000)%
         %send% %actor% %self.name% gives you %payout% coins.
         %echoaround% %actor% %self.name% gives %actor.name% a large pile of gold coins.
      elseif %wager% == 1000000
         nop %actor.gold(2900000)%
         %send% %actor% %self.name% gives you %payout% coins.
         %echoaround% %actor% %self.name% gives %actor.name% a large pile of gold coins.
      end
   unset wager
   unset payout
elseif (%position% == 2 && %guess% == 2)
   tell %actor.name% You have wagered %wager% coins on the %location% shell.
   wait 5
   tell %actor.name% You have chosen wisely.
      if %wager% == 1000
         nop %actor.gold(2900)%
         %send% %actor% %self.name% gives you %payout% coins.
         %echoaround% %actor% %self.name% gives %actor.name% a large pile of gold coins.
      elseif %wager% == 10000
         nop %actor.gold(29000)%
         %send% %actor% %self.name% gives you %payout% coins.
         %echoaround% %actor% %self.name% gives %actor.name% a large pile of gold coins.
      elseif %wager% == 100000
         nop %actor.gold(290000)%
         %send% %actor% %self.name% gives you %payout% coins.
         %echoaround% %actor% %self.name% gives %actor.name% a large pile of gold coins.
      elseif %wager% == 1000000
         nop %actor.gold(2900000)%
         %send% %actor% %self.name% gives you %payout% coins.
         %echoaround% %actor% %self.name% gives %actor.name% a large pile of gold coins.
      end
   unset wager
   unset payout
elseif (%position% == 3 && %guess% == 3)
   tell %actor.name% You have wagered %wager% coins on the %location% shell.
   wait 5
   tell %actor.name% You have chosen wisely.
      if %wager% == 1000
         nop %actor.gold(2900)%
         %send% %actor% %self.name% gives you %payout% coins.
         %echoaround% %actor% %self.name% gives %actor.name% a large pile of gold coins.
      elseif %wager% == 10000
         nop %actor.gold(29000)%
         %send% %actor% %self.name% gives you %payout% coins.
         %echoaround% %actor% %self.name% gives %actor.name% a large pile of gold coins.
      elseif %wager% == 100000
         nop %actor.gold(290000)%
         %send% %actor% %self.name% gives you %payout% coins.
         %echoaround% %actor% %self.name% gives %actor.name% a large pile of gold coins.
      elseif %wager% == 1000000
         nop %actor.gold(2900000)%
         %send% %actor% %self.name% gives you %payout% coins.
         %echoaround% %actor% %self.name% gives %actor.name% a large pile of gold coins.
      end
   give %actor.name% %wager% coins
   unset wager
   unset payout
elseif (%position% == 1 && %guess% != 1)
   tell %actor.name% You have wagered %wager% coins on the %location% shell.
   wait 5
   tell %actor.name% So sorry, but the bean was on the Left.
   unset wager
   unset payout
elseif (%position% == 2 && %guess% != 2)
   tell %actor.name% You have wagered %wager% coins on the %location% shell.
   wait 5
   tell %actor.name% So sorry, but the bean was in the middle.
   unset wager
   unset payout
elseif (%position% == 3 && %guess% !=3)
   tell %actor.name% You have wagered %wager% coins on the %location% shell.
   wait 5
   tell %actor.name% So sorry, but the bean was on the right.
   unset wager
   unset payout
end
~
$~
