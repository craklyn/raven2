#37101
Bad Choice~
1 c 100
d~
if %cmd.mudcommand% == drink && 'fountain /= '%arg% && %arg.strlen% > 0
  %send% %actor% That wasn't such a good idea.
  %echoaround% %actor% %actor.name% has made a terrible mistake and drinks from the fountain.
  wait 1 sec
  %echoaround% %actor% Unable to scream, %actor.name% looks at you with pleading eyes as they erupt into flames.
  %damage% %actor% 5000
else
  return 0
end
~
#37104
Bad Choice 2~
1 c 100
s~
if %cmd.mudcommand% == sip && 'fountain /= '%arg% && %arg.strlen% > 0
  %send% %actor% That wasn't such a good idea.
  %echoaround% %actor% %actor.name% has made a terrible mistake and sips from the fountain.
  wait 1 sec
  %echoaround% %actor% Unable to scream, %actor.name% looks at you with pleading eyes as they erupt into flames.
  %damage% %actor% 5000
else
  return 0
end
~
#37105
Coin/Spark Redemption~
0 j 100
~
if %object.vnum% == 37150
  wait 1 sec
  %echo% Ichiga says, 'You wish to redeem your coin! Give it to my assistant.'
  wait 1 sec
  %echo% The assistant groans loudly.
  wait 2 sec
  %echo% Ichiga makes a strange gesture and a small coin engraved with an "&10E&00" flies over to his assistant.
  %purge% coin
  wait 2 sec
  %echo% The assistant quivers and spits out a pouch.
  wait 1 sec
  %echo% Ichiga takes a wildflower, a bright pink potion, and a small secret out the pouch.
  wait 1 sec
  %echo% Ichiga mixes all three in a wooden bowl.
  wait 3 sec
  %echo% A &14BRIGHT&00 flash of light fills the room.
  %echo% You are momentarily blinded!
  wait 2 sec
  %echo% The assistant trembles and lets out a small shriek.
  wait 1 sec
  %echo% Ichiga says, 'These so called protectors have been allowing the raiders of the north to pass through our city at night.'
  wait 1 sec
  %echo% Ichiga says, 'This may not win the war...but it shall suffice. Thank you again, %actor.name%.'
  wait 1 sec
  %load% obj 37151
  %load% obj 37151
  %load% obj 37151
  %echo% The assistant spits out a sputtering potion!
  %echo% The assistant spits out a sputtering potion!
  %echo% The assistant spits out a sputtering potion!
  wait 1 sec
  %echo% The assistant screams and explodes into non-existence!
  %purge% assistant
elseif %object.vnum% == 37183
  wait 1 sec
  %echo% Ichiga says, 'You'll wish you had accepted that in the first place.'
  wait 1 sec
  %echo% The assistant starts to shiver.
  wait 2 sec
  %echo% Ichiga makes a strange gesture and a small spark flies over to the decrepit assistant.
  %purge% spark
  wait 2 sec
  %echo% The assistant starts to tremble with absolute fear.
  wait 1 sec
  %echo% The assistant screams and spits out a small pouch.
  wait 1 sec
  %echo% Ichiga gets some items out of the small pouch and crushes them in his hands.
  wait 3 sec
  %echo% A large &10f&08l&10a&08m&10e&00 comes down from the heavens!
  %echo% You hear the deafening ROAR of a dragon!
  wait 2 sec
  %echo% The assistant quivers and starts to shriek.
  wait 1 sec
  %echo% Ichiga looks at the assistant and sneers.
  wait 1 sec
  %echo% Ichiga flicks his wrist at the assistant.
  wait 1 sec
  %load% obj 37184
  %load% obj 37184
  %load% obj 37184
  %echo% The assistant spits out a small &10f&08l&10a&08m&10e&00!
  %echo% The assistant spits out a small &10f&08l&10a&08m&10e&00!
  %echo% The assistant spits out a small &10f&08l&10a&08m&10e&00!
  wait 1 sec
  %echo% The assistant screams and explodes into non-existence!
  %purge% assistant
end
~
#37106
DONE~
0 a 100
~
if %hasran%
  unset hasran
end
~
#37107
Coin2 Redemption~
0 j 100
~
if %object.vnum% == 37180
  wait 1 sec
  %echo% Ichiga says, 'Nonsense! Accept his coin you worthless trash!'
  wait 1 sec
  %echo% The slave groans loudly.
  wait 2 sec
  %echo% Ichiga makes a strange gesture and a small token engraved with an "&10E&00" flies over to his slave.
  %purge% token
  wait 2 sec
  %echo% The beaten slave begins to writhe in pain.
  wait 1 sec
  %echo% Ichiga grabs a pitch black samurai sword and a &00&06moonlight potion&00 from a table.
  wait 1 sec
  %echo% Ichiga slathers the potion on the sword and grabs a potion of safety from a small box.
  wait 3 sec
  %echo% A wave of complete &07darkness&00 washes the room.
  %echo% The room goes black.
  wait 2 sec
  %echo% The slave screams and begins to twitch.
  wait 1 sec
  %echo% Ichiga says, 'These items when combined create a powerful tool that I can use against the traitors here.'
  wait 1 sec
  %echo% Ichiga says, 'You may take them to assist with whatever you need. Thank you again, %actor.name%.'
  wait 1 sec
  %load% obj 37152
  %load% obj 37152
  %load% obj 37152
  %echo% The slave spits out a dusty scroll!
  %echo% The slave spits out a dusty scroll!
  %echo% The slave spits out a dusty scroll!
  wait 1 sec
  %echo% The slave screams and explodes all over the room!
  %purge% slave
done
~
#37108
Coin3 Redemption~
0 j 100
~
if %object.vnum% == 37181
  wait 1 sec
  %echo% Ichiga says, 'My dear. We have company. Please treat them with respect.'
  wait 1 sec
  %echo% The mistress smiles and says, 'Yes, master.'
  wait 2 sec
  %echo% Ichiga makes a strange gesture and a small rock engraved with an "&10E&00" flies over to his mistress.
  %purge% stone
  wait 2 sec
  %echo% The mistress places the stone in her pocket.
  wait 1 sec
  %echo% Ichiga grabs a fizzing potion and a glowing blue vial from the table.
  wait 1 sec
  %echo% Ichiga quickly puts the fizzing potion in the vial and adds some fungus and some spices.
  wait 3 sec
  %echo% A pleasant aroma fills the room.
  wait 2 sec
  %echo% The mistress giggles and reaches into her pocket.
  wait 1 sec
  %echo% Ichiga says, 'My lovely mistress will now give you your reward.'
  wait 1 sec
  %echo% Ichiga says, 'Please, come back! Thank you again, %actor.name%.'
  wait 1 sec
  %load% obj 37182
  %load% obj 37182
  %load% obj 37182
  %echo% The mistress pulls forth a &07pitch black&00 potion!
  %echo% The mistress pulls forth a &07pitch black&00 potion!
  %echo% The mistress pulls forth a &07pitch black&00 potion!
  wait 1 sec
  %echo% The mistress drops the potions on the ground.
  wait 1 sec
  %echo% Ichiga says, 'My dear. We need some supplies. Mind getting them?'
  wait 1 sec
  %echo% Ichiga flicks his wrist and the mistress disappears.
  %purge% mistress
done
~
#37131
Seagulls~
2 ag 100
~
if %371gullshasran%
  wait 1 sec
  switch %random.05%
    case 1
      %echo% The sound of gulls can be heard in the distance.
    break
    case 2
      %echo% A flock of gulls fly lazily overhead.
    break
    case 3
      %echo% The squawking and cawing of different species of birds can be heard in the distance. 
    break
    case 4
      %echo% A slight breeze blows in and fills the air with a light mist of ocean spray.
    break
    case 5
      %echo% A steady breeze has kicked up here, blowing around some trash in the street.
    break
    default
      %echo% The air is very still here.
    break
  done
else
  wait 1 sec
  switch %random.10%
    case 1
      %echo% A flock of screeching gulls swoop in and begin foraging the ground around your feet for food.
    break
    case 2
      %echo% A group of birds flies in over the wall in hopes of scrounging for bits of food.
    break
    case 3
      %echo% A large flock of gulls stare up at you as if waiting to be fed.
    break
    case 4
      %echo% A group of gulls streak in from the sky take up refuge on a nearby bench.
    break
    case 5
      %echo% A screeching flock of gulls fly in from north in search of some food.
    break
    case 6
      %echo% A large flock of gulls descend on you in hopes that you have some food.
    break
    case 7
      %echo% As a passing gull drops a freshly caught fish, a larger group following it quickly descend on the potential meal.
    break
    case 8
      %echo% As you enter the room you hear nothing but the loud screeching of a flock of gulls.
    break
    case 9
      %echo% A group of gulls emerge from a nearby side street.
    break
    case 10
      %echo% A screeching flock of gulls has taken a sudden interest in you and begin to follow you.
    break
    default
      %echo% Some birds have landed here, intent on foraging for food.
    break
  done
  %load% mob 37150
  set 371gullshasran 1
  global 371gullshasran
end
~
#37132
Zone Reset - Return Gulls~
2 f 100
~
if %371gullshasran%
  unset 371gullshasran
end
~
#37133
Northern Road - Environment~
2 g 100
~
wait 1 sec
switch %random.10%
  case 1
    %echo% Dark smoke seems to choke the oxygen out of the air here.
  break
  case 2
    %echo% The ground shakes as a mountain to the north begins to crumble.
  break
  case 3
    %echo% A large drake circles high above you.
  break
  case 4
    %echo% Grey in color, ashes begin to flake like a winter snow.
  break
  case 5
    if %371lighthasran%
      wait 1 sec
      %echo% Flashes of lightning pock the landscape to the north.
    else
      %echo% A large &14**&10C&14RA&10C&14K**&00 followed by a white flash of light strikes the path in front of you!
      %echo% You are momentarily blinded!
      %load% mob 37133
      set 371lighthasran 1
      global 371lighthasran
    end
  break
  case 6
    %echo% Thunder from the north breaks the eerie silence.
  break
  case 7
    %echo% As %actor.name% enters the room, a large chuck of black rock comes screaming in from the north and hits them in the side with a dull THUD.
    %damage% %actor% 125
  break
  case 8
    %echo% A large mountain to the north begins to erupt.
  break
  case 9
    %echo% A large drake swoops in and tries to snatch you up for a quick meal! Lucky for you it missed.
  break
  case 10
    %echo% The smoke seems to be a bit thicker here. You are having trouble breathing.
    wait 1 sec
    %echo% You begin to choke and gag.
  break
  default
    %echo% The air is very warm here.
  break
done
~
#37134
Zone Reset - Lightning Elemental~
2 f 100
~
if %371lighthasran%
  unset 371lighthasran
end
~
#37135
Fire Drake~
2 g 100
~
if %371drakehasran%
  wait 1 sec
  switch %random.05%
    case 1
      %echo% The screeching of squabbling fire drakes can be heard to the north.
    break
    case 2
      %echo% Fire drakes can be seen patrolling the skies to the north.
    break
    case 3
      %echo% A breeze from the north carries a warm wind.
    break
    case 4
      %echo% The wind from the north smells of ash and sulphur.
    break
    case 5
      %echo% A breeze picks up and blows in grey ash from the north.
    break
    default
      %echo% A large fire drake passes overhead.
    break
  done
else
  wait 1 sec
  switch %random.10%
    case 1
      %echo% A large fire drake descends upon you, screeching.
    break
    case 2
      %echo% A large drake has noticed you from above.
      wait 1 sec
      %echo% The drake *THUMPS* down in front of you.
    break
    case 3
      %echo% A large fire drake emerges from a side street.
    break
    case 4
      %echo% A sudden strong wind is kicked up as a fire drake lands behind you.
    break
    case 5
      %echo% The screeching is all you hear as a fire drake decides to set down beside you.
    break
    case 6
      %echo% A large drake has taken refuge here on the northern wall. It hops down as you approach.
    break
    case 7
      %echo% You watch in awe as a female fire drake slowly descends into the room. She screeches loudly.
    break
    case 8
      %echo% A fire drake hops out of a nearby alley and screeches at you.
    break
    case 9
      %echo% Out of the shadows steps a large fire drake. She has what remains of an elemental guard in her jaws.
    break
    case 10
      %echo% A fire drake lands a ways away from you. She is seemingly interested in other things.
    break
    default
      %echo% A fire drake lands in front of you. *THUMP*
    break
  done
  %load% mob 37151
  set 371drakehasran 1
  global 371drakehasran
end
~
#37136
Zone Reset - Drake~
2 f 100
~
if %371drakehasran%
  unset 371drakehasran
end
~
#37150
Gulls Respawn~
2 f 100
~
%load% mob 37150
~
#37177
Ichiga Greeting Reset~
2 g 100
~
if %ichigahasran%
  unset ichigahasran
end
~
#37182
Greta Entrance~
0 h 100
~
if (%direction% == west)
  wait 1 sec
  switch %random.10%
    case 1
      %send% %actor% Greta smiles as you enter her shop.
      %echoaround% %actor% Greta smiles as %actor.name% enters the shop.
      wait 1 sec
      %send% %actor% Greta asks, "Have you brought me anything good?"
    break
    case 2
      %send% %actor% A small elemental notices your entrance.
      %echoaround% %actor% A small elemental notices %actor.name% enter the shop.
      wait 1 sec
      %send% %actor% Greta giggles and nods.
    break
    case 3
      %send% %actor% Greta looks at you.
      %echoaround% %actor% Greta looks at %actor.name%.
      wait 1 sec
      %send% %actor% Greta winks at you.
      %echoaround% %actor% Greta winks at %actor.name%.
    break
    case 4
      %send% %actor% A small machine bumps into your foot.
      %echoaround% %actor% A small machine bumps into the foot of %actor.name%.
      wait 1 sec
      %send% %actor% Greta apologizes sincerely. "I never should have bought that piece of junk."
      %echoaround% %actor% Greta apologizes sincerely.
    break
    case 5
      %send% %actor% Greta is just finishing up with another customer as you walk in.
      %echoaround% %actor% Greta notices %actor.name% and will be with them in a minute.
      wait 1 sec
      %send% %actor% Greta apologizes and says, "Now, how may I help you?"
      %echoaround% %actor% Greta apologizes sincerely.
    break
    case 6
      %send% %actor% A small mass of light grey onyx and vegetation morphs into something you can understand.
      %echoaround% %actor% Greta takes on a form that %actor.name% can understand.
    break
    case 7
      %send% %actor% Greta winks and beckons you over to her.
      %echoaround% %actor% Greta beckons %actor.name% over to the counter.
    break
    case 8
      %send% %actor% A small mass light grey onyx and vegetation morphs into something you can understand.
      %echoaround% %actor% Greta nods at %actor.name% as they enter the shop.
      wait 1 sec
      %send% %actor% Greta asks, "How can I be of assistance?"
    break
    case 9
      %send% %actor% Greta sighs and puts down a picture.
      %echoaround% %actor% Greta sighs.
      wait 1 sec
      %send% %actor% Greta wipes away a tear and beckons you forward.
      %echoaround% %actor% Greta sniffs loudly.
    break
    case 10
      %send% %actor% Greta is behind the counter and staring blankly into space.
      %echoaround% %actor% Greta doesn't seem to notice %actor.name% enter the shop.
    break
    default
      %send% %actor% Greta beckons you over to her.
    break
  done
end
~
#37183
Reorx Entrance~
0 h 100
~
if (%direction% == east)
  wait 1 sec
  switch %random.10%
    case 1
      %send% %actor% Reorx puts down his hammer and beckons you in.
      %echoaround% %actor% Reorx puts down his hammer and beckons %actor.name% over to the counter.
      wait 1 sec
    break
    case 2
      %send% %actor% As you enter the shop a large mass of onyx and vegetation morphs into something you can understand.
      %echoaround% %actor% Reorx greets %actor.name% as they enter the shop.
    break
    case 3
      %send% %actor% A large elemental notices your entrance.
      %echoaround% %actor% Reorx looks at %actor.name%.
      wait 1 sec
      %send% %actor% Reorx winks at you.
      %echoaround% %actor% Reorx winks at %actor.name%.
    break
    case 4
      %send% %actor$ The pit of lava bubbles and hisses as Reorx plunges a sword into it.
      %echoaround% %actor% The pit of lava bubbles and hisses as Reorx plunges a sword into it.
    break
    case 5
      %send% %actor% The sound of hissing water hits your ears.
      %echoaround% %actor% Reorx is too busy to notice %actor.name% enter the shop.
      wait 1 sec
      %send% %actor% Reorx apologizes and says, "Now, how may I help you?"
      %echoaround% %actor% Reorx apologizes sincerely.
    break
    case 6
      %send% %actor% Reorx grumbles as you disturb him.
      %echoaround% %actor% Reorx glares at %actor.name% as they enter his shop.
    break
    case 7
      %send% %actor% A large elemental is peering into a pool of lava.
      %echoaround% %actor% Reorx stops what he is doing to check on a recently dipped weapon.
    break
    case 8
      %send% %actor% Reorx gives you a strange look.
      %echoaround% %actor% Reorx gives %actor.name% a strange look.
      wait 1 sec
      %send% %actor% Reorx asks, "How can I be of assistance?"
    break
    case 9
      %send% %actor% The elemental behind the counter beckons you in.
      %echoaround% %actor% Reorx beckons %actor.name% over to the counter.
    break
    case 10
      %send% %actor% The large elemental takes a vested interest in you.
      %echoaround% %actor% %actor.name% enters the shop and Reorx immediately takes notice.
    break
    default
      %send% %actor% Reorx beckons you over to her.
    break
  done
end
~
#37184
Blackclaw Entrance~
0 h 100
~
if (%direction% == west)
  wait 1 sec
  switch %random.10%
    case 1
      %send% %actor% Blackclaw grunts as you enter his shop.
      %echoaround% %actor% Blackclaw grunts as new customers enter his shop.
    break
    case 2
      %send% %actor% Blackclaw seems to take a vested interest in you as you enter.
      %echoaround% %actor% Seems to be interested in what %actor.name% has to offer.
      wait 1 sec
      %send% %actor% "May I help you?" he grunts.
      %echoaround% %actor% Blackclaw beckons %actor.name% over to the counter.
    break
    case 3
      %send% %actor% Blackclaw stops what he was doing and fixates his eyes on you.
      %echoaround% %actor% The shopkeeper keeps a wary eye on %actor.name%.
    break
    case 4
      %send% %actor% Blackclaw beckons you over to the counter.
      %echoaround% %actor% Blackclaw waves %actor.name% over to him.
    break
    case 5
      %send% %actor% A large mass of moving earth and onyx morphs into something you can understand.
      %echoaround% %actor% Blackclaw quickly morphs into something understandable.
      wait 1 sec
      %send% %actor% Blackclaw says, "Welcome. Please feel free to look around."
      %echoaround% %actor% Blackclaw welcomes you to his shop.
    break
    case 6
      %send% %actor% Blackclaw looks extremely busy at the moment.
      %echoaround% %actor% Frantically searching for something, Blackclaw barely notices %actor.name% enter the shop.
      wait 1 sec
      %send% %actor% Blackclaw twiddles his thumbs and hums.
      %echoaround% %actor% Blackclaw twiddles his thumbs.
    break
    case 7
      %send% %actor% The large elemental behind the counter shifts his gaze to you.
      %echoaround% %actor% As %actor.name% enters the shop Blackclaw directs his gaze towards them.
    break
    case 8
      %send% %actor% As you enter the shop, Blackclaw is just finishing up with another customer.
      %echoaround% %actor% Blackclaw bids a current customer farewell.
      wait 1 sec
      %send% %actor% Blackclaw says, "Yes, welcome."
      %echoaround% %actor% He bids welcome to %actor.name%.
    break
    case 9
      %send% %actor% Blackclaw is just about to fall asleep when you walk in the room.
      %echoaround% %actor% The shopkeeper starts to nod off.
      wait 1 sec
      %send% %actor% Blackclaw coughs loudly.
      %echoaround% %actor% Blackclaw coughs very loudly.
    break
    case 10
      %send% %actor% Blackclaw looks displeased now that you have disturbed him.
      %echoaround% %actor%  The shopkeeper seems to get irritated with the entrance of ANOTHER customer.
    break
    default
      %send% %actor% Blackclaw beckons you over to the counter.
    break
  done
end
~
#37185
Roxanne Entrance~
0 h 100
~
if (%direction% == east)
  wait 1 sec
  switch %random.10%
    case 1
      %send% %actor% Roxanne seems surprised when you walk into the shop. 
      %echoaround% %actor% Roxanne looks surprised to see a customer.
    break
    case 2
      %send% %actor% Roxanne says, "I hope you have brought me something good."
      %echoaround% %actor% Roxanne looks eager to purchase some goods.
      wait 1 sec
      %send% %actor% Roxanne licks her mouth and smiles.
    break
    case 3
      %send% %actor% Roxanne looks up as you enter her shop.
      %echoaround% %actor% Roxanne looks up as %actor.name% enters her shop.
    break
    case 4
      %send% %actor% Roxanne looks up from her work and eyeballs you suspiciously.
      %echoaround% %actor% You notice Roxanne is eyeballing %actor.name% suspiciously.
      wait 1 sec
      %send% %actor% Roxanne says, "Can I help you with something?"
      %echoaround% %actor% Roxanne asks %actor.name% a question.
    break
    case 5
      %send% %actor% A large machine on the wall of the shop begins to turn.
      %echoaround% %actor% Machines on the wall alert Roxanne to the presence of customers.
      wait 1 sec
      %send% %actor% Roxanne looks up from her work to greet you.
      %echoaround% %actor% Roxanne looks up from her work to greet %actor.name%.
    break
    case 6
      %send% %actor% You notice that Roxanne is eyeballing you suspiciously.
      %echoaround% %actor% Roxanne gives %actor.name% a strange look.
    break
    case 7
      %send% %actor% "Ah, a customer. Haven't had one in days!" says Roxanne as you enter her shop.
      %echoaround% %actor% Roxanne seems to be excited to have a customer.
    break
    case 8
      %send% %actor% The shopkeeper looks at you eagerly as you enter her shop.
      %echoaround% %actor% The shopkeeper looks at %actor.name% eagerly.
      wait 1 sec
      %send% %actor% Roxanne asks, "May I help you?"
      %echoaround% %actor% Roxanne asks, "May I help you %actor.name%?"
    break
    case 9
      %send% %actor% Roxanne looks at you and smiles. Then quietly returns to her work.
      %echoaround% %actor% Roxanne doesn't seem to notice a customer enter her shop.
    break
    case 10
      %send% %actor% Roxanne snickers when you walk into the shop.
      %echoaround% %actor% Roxanne looks at %actor.name%.
      wait 1 sec
      %echoaround% %actor% Roxanne snickers.
    break
    default
      %send% %actor% Roxanne beckons you over to the counter.
    break
  done
end
~
#37186
Kiki's Entrance~
0 h 100
~
if (%direction% == west)
  wait 1 sec
  switch %random.10%
    case 1
      %send% %actor% Kiki looks up from her book and says, "Welcome".
      %echoaround% %actor% Kiki looks up from her book and says, "Welcome".
    break
    case 2
      %send% %actor% Kiki puts her book down and says, "Please, see what I have in stock".
      %echoaround% %actor% Kiki looks up from her book as %actor.name% enters the shop.
    break
    case 3
      %send% %actor% "Welcome to my shop! Please feel free to browse."
      %echoaround% %actor% Kiki welcomes %actor.name% to her shop.
      wait 1 sec
      %send% %actor% Kiki giggles.
    break
    case 4
      %send% %actor% Kiki looks at you with earnest wondering if you have brought anything to sell. 
      %echoaround% %actor% A look of interest has spread of the face of Kiki.
    break
    case 5
      %send% %actor% Kiki smiles as you enter and puts down the book she was reading.
      %echoaround% %actor% smiles as %actor.name% approaches her counter.
    break
    case 6
      %send% %actor% Kiki chuckles as you enter the shop. "Are you used to the sulfur yet?"
      %echoaround% %actor% Kiki asks %actor.name% a question.
    break
    case 7
      %send% %actor% As you step into the shop, Kiki hardly notices and continues to read her book.
      %echoaround% %actor% Kiki doesn't notice the new customers that are now in her shop.
    break
    case 8
      %send% %actor% Kiki smiles and beckons you in with a slight nod.
      %echoaround% %actor% Kiki smiles and nods.
      wait 1 sec
      %send% %actor% Kiki waves you over to her.
      %echoaround% %actor% Kiki waves for %actor.name% to approach her.
    break
    case 9
      %send% %actor% Kiki licks her chops and smiles as you enter the shop.
      %echoaround% %actor% Kiki licks her mouth and smiles.
    break
    case 10
      %send% %actor% Kiki notices your entrance and shoots you a quick once over.
      %echoaround% %actor% Kiki gives %actor.name% a quick once over.
    break
    default
      %send% %actor% Kiki beckons you over to the counter.
    break
  done
end
~
#37187
Crysto's Entrance~
0 h 100
~
if (%direction% == east)
  wait 1 sec
  switch %random.10%
    case 1
      %send% %actor% A mix of dust and magic EXPLODES in Crysto's face. He is not pleased.
      %echoaround% %actor% Crysto scoffs loudly when a mixture of dust and magic explodes in his face.
    break
    case 2
      %send% %actor% As you enter the room, Crysto notices you, nods, and mumbles something unintelligible.
      %echoaround% %actor% Crysto mumbles something as %actor.name% approaches the counter.
    break
    case 3
      %send% %actor% As you enter the shop, Crysto points to the "Prices are FINAL" sign hanging from the front of the counter.
      %echoaround% %actor% As %actor.name% approaches the counter, Crysto mumbles something and points to a small sign.
    break
    case 4
      %send% %actor% Crysto notices you enter the shop and immediately proceeds to pretend to look busy.
      %echoaround% %actor% Crysto pretends to look busy when %actor.name% advances towards the counter.
    break
    case 5
      %send% %actor% Crysto scoffs and says, "Pfft. Didn't think I'd have to see YOU out here."
      %echoaround% %actor% Crysto scoffs and makes a snide remark to %actor.name%.
      wait 1 sec
      %send% %actor% Crysto snickers softly.
      %echoaround% %actor% Crysto snickers.
    break
    case 6
      %send% %actor% Crysto is asleep at the counter. A bottle of booze has spilled as well.
      %echoaround% %actor% %actor.name% kicks over an empty bottle of Rum and they enter the shop.
    break
    case 7
      %send% %actor% As you enter the shop you see Crysto frantically searching for something.
      %echoaround% %actor% Crysto is searching for something and doesn't notice %actor.name% enter the room.
    break
    case 8
      %send% %actor% Crysto says, "Welcome to my shop. Please free feel to look around."
      %echoaround% %actor% Crysto mumbles something as %actor.name% stumbles into the shop.
      wait 1 sec
      %send% %actor% Crysto hiccups.
      %echoaround% %actor% Crysto hiccups loudly.
    break
    case 9
      %send% %actor% Crysto giggles and whispers, "Have you come to suckle on the merchant's teat?"
      %echoaround% %actor% Crysto giggles and whispers something to %actor.name%.
    break
    case 10
      %send% %actor% Crysto stumbles over something as you have startled him. He looks embarrassed.
      %echoaround% %actor% Crysto trips over an empty bottle of rum. He looks quite embarrassed.
    break
    default
      %send% %actor% Crysto beckons you over to the counter.
    break
  done
end
~
#37188
Selka Entrance~
0 g 100
~
if (%direction% == west)
  wait 1 sec
  switch %random.10%
    case 1
      %send% %actor% Selka glances up from her work and smiles
      wait 1 sec
      %send% %actor% "Welcome! Are you travelling north into the mountains?"
      %echoaround% %actor% Selka glances up from her work and smiles.
    break
    case 2
      %send% %actor% A mass of water morphs into something you can understand.
      %echoaround% %actor% Selka takes on a form that %actor.name% can understand.
      wait 1 sec
      %send% %actor% Selka laughs and says, "Welcome traveler. How may I help you?"
      %echoaround% %actor% Selka chuckles and begins to converse with %actor.name%.
    break
    case 3
      %send% %actor% Selka is here playing with the water.
      %echoaround% %actor% Selka splashes some water at %actor.name% as they enter.
      wait 1 sec
      %send% %actor% Selka waves you in with a smile.
      %echoaround% %actor% Selka waves %actor.name% in.
    break
    case 4
      %send% %actor% Selka looks displeased as you have disturbed her play time.
      %echoaround% %actor% Selka grunts as %actor.name% has interrupted her play time.
      wait 1 sec
      %send% %actor% Selka frowns. What's wrong?
      %echoaround% %actor% Selka frowns. What's wrong?
    break
    case 5
      %send% %actor% As you enter the room, a small elemental takes interest.
      %echoaround% %actor% Selka takes interest as a new customer enters the shop.
      wait 1 sec
      %send% %actor% She giggles and beckons you closer.
    break
    case 6
      %send% %actor% Selka takes a break from bottling water and notices you have entered.
      %echoaround% A busy elemental takes a break from bottling water.
      wait 1 sec
      %send% %actor% Selka says, "How may I be of service?"
      %echoaround% %actor% Selka asks %actor.name% a question.
    break
    case 7
      %send% %actor% Selka points to the disclaimer on the wall.
      %echoaround% %actor% The shopkeeper points to a disclaimer on the wall.
      wait 1 sec
      %send% %actor% "Please read it before you purchase anything" she says.
      %echoaround% %actor% Selka mutters something about reading it...
    break
    case 8
      %send% %actor% Selka is busy reading her inventory list and couldn't be bothered with you.
      %echoaround% %actor% Selka fails to notice %actor.name% enter the shop.
      wait 1 sec
      %send% %actor% She says, "Just take what you need and leave gold on the counter."
      %echoaround% %actor% She mentions something about gold and leaving it on the counter.
    break
    case 9
      %send% %actor% Selka looks up at you as you enter the shop.
      %echoaround% %actor% Selka looks at %actor.name%.
      wait 1 sec
      %send% %actor% Selka snickers quietly.
      %echoaround% %actor% Selka snickers.
    break
    case 10
      %send% %actor% Selka beckons you to come in.
      %echoaround% %actor% Selka beckons %actor.name% over to the counter.
    break
    default
      %send% %actor% Selka beckons you over to the counter.
    break
  done
end
~
#37189
Katsumi Entance~
0 h 100
~
if (%direction% == east)
  wait 1 sec
  switch %random.10%
    case 1
      %send% %actor% Katsumi floats over to you and offers to take your coat.
      %echoaround% %actor% Katsumi offers to take %actor.name%'s coat as they enter the stop.
    break
    case 2
      %send% %actor% Katsumi is floating between the stock, checking for items.
      %echoaround% %actor% Katsumi glances at her list and begins to check off items.
    break
    case 3
      %send% %actor% A small transparent mass of onyx and vegetation morphs into something you can understand.
      %echoaround% %actor% Katsumi nods as %actor.name% enters her shop.
      wait 1 sec
      %send% %actor% Katsumi giggles and asks, "What can I help you with?"
    break
    case 4
      %send% %actor% A small elemental directs its gaze to you as you enter the shop.
      %echoaround% %actor% Katsumi seems to gaze upon %actor.name% as they enter the shop.
    break
    case 5
      %send% %actor% Katsumi is floating behind the counter and reading her inventory list.
      %echoaround% %actor% As %actor.name% enters the room Katsumi glances up from her inventory list.
      wait 1 sec
      %send% %actor% She waves you in.
      %echoaround% %actor% She waves %actor.name% in.
    break
    case 6
      %send% %actor% As you enter the shop a small elemental seems to takes interest in you.
      %echoaround% %actor% Katsumi seems to be interested in what %actor.name% is doing here.
      wait 1 sec
      %send% %actor% Katsumi says, "Welcome!"
      %echoaround% %actor% Katsumi says, "Welcome!"
    break
    case 7
      %send% %actor% A small elemental floating here seems displeased with your sudden entrance.
      %echoaround% %actor% Katsumi glares at %actor.name% as they enter the shop, obviously displeased.
    break
    case 8
      %send% %actor% Katsumi puts down her list and smiles. "Welcome!"
      %echoaround% %actor% Katsumi stops what she is doing to bid welcome to %actor.name%.
    break
    case 9
      %send% %actor% Katsumi smiles and says, "I'll be with you in a moment."
      %echoaround% %actor% Katsumi is reading something and puts her hand up as %actor.name% approaches the counter.
    break
    case 10
      %send% %actor% Katsumi is just finishing up with another customer as you enter.
      %echoaround% %actor% Another customer is leaving the shop when Katsumi beckons %actor.name% to approach.
      wait 1 sec
      %send% %actor% Katsumi says, "How may I be of assistance?"
      %echoaround% %actor% Katsumi asks %actor.name% a question.
    break
    default
      %send% %actor% Katsumi beckons you over to the counter.
    break
  done
end
~
$~
