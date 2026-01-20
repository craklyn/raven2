#38800
thief guardian script --> m38800~
0 e 100
nods in agreement with you.~
if (%actor.is_npc% != 0 || %actor.level% > 50)
      halt
elseif (%actor.class% == Thief)
         tell %actor.name% You've got good taste.  Come this way.
         %echoaround% %actor.name% %self.name% quickly opens a trap door, shoves %actor.name% through, and closes it again.
         %send% %actor.name% %self.name% quickly opens a trap door and pushes you through.  He closes it behind you.
         %teleport% %actor.name% 38806
         %force% %actor% look
         %send% %actor% You stumble down the stairs and barely catch yourself at the bottom.
       else
   tell %actor.name% Aye, ye've got good taste, but ye have no business being here! Get out of my kitchen!
         open door west
         %echoaround% %actor.name% %self.name% shoves %actor.name% out of the kitchen.
         %send% %actor.name% %self.name% shoves you out of the kitchen.
         %force% %actor.name% west
         close door west
       end
~
#38801
cook greeting --> m38800~
0 g 100
~
if (%actor.is_npc% != 0 || %actor.level% > 50)
      halt
   else
say I think this sauce needs a little whiskey in it. What do you think?
end
~
#38802
Cook Echos for 38800~
0 b 20
~
eval option %random.5%
   switch %option%
   case 1
      %echo% As flame erupts from a nearby burner, %self.name% roars in rage and rushes over to subdue the cooking.
      break
   case 2
      %echo% Cheers erupt from nearby cooks as the Master Cook taste-tests a sauce and breaks into a smile.
      break
   case 3
      %echo% %self.name% grimaces as he tastes a sauce and blurts out, "More garlic! More salt! Move it!"
      break
   case 4
      %echo% %self.name% shuffles around the kitchen, his enormous girth clogging up an entire aisle.
      break
   case 5
      %echo% Several cooks bow their heads in slight shame as %self.name% tastes one of their sauces and frowns.
      break
done
~
#38803
Waiter script for mobile 38805~
0 b 35
~
   eval option %random.5%
   switch %option%
   case 1
      say Excuse me, coming through.
      break
   case 2
      say Watch out there, I've got a plate of hot and delicious food!
      break
   case 3
      %echo% Making his way through the crowd, %self.name% laughs at a nearby comment about him.
      break
   case 4
      %echo% Stopping by a table, %self.name% refills a drink.
      break
   case 5
      %echo% %self.name% delivers a platter of dishes to a table of hungry patrons.
      break
   done
~
#38804
Echo script for Bard vnum 38804~
0 b 40
~
   eval song %random.5%
   switch %song%
   case 1
      %echo% A happy tune wafts through the restaurant as %self.name% drunkenly strums out a song.
      break
   case 2
      %echo% Breaking into fits of laughter, %self.name% stops singing to drink some beer.
      break
   case 3
      %echo% A happy tune wafts through the restaurant as %self.name% drunkenly strums out a song.
      break
   case 4
      %echo% A happy tune wafts through the restaurant as %self.name% drunkenly strums out a song.
      break
   case 5
      %echo% A happy tune wafts through the restaurant as %self.name% drunkenly strums out a song.
      break
   done
~
#38805
Test trigger for gnome scopes~
1 c 100
shrink~
   eval item %actor.eq(hold)%
   if (%item.vnum% == 9598)
   %echoaround% %actor% %actor.name% suddenly shrinks to microscopic size and jumps through the spy glass.
   %send% %actor% You feel yourself shrinking rapidly.  You jump towards the spy glass and find you are small enough to fit through it.
   %teleport% %actor% 9522
   %force% %actor% look
   else
   %echoaround% %actor% %actor.name% tries to enter the portal but cracks his head on the side.
   %send% %actor% You crack your head open trying to enter the portal.
   %damage% %actor% 50
   end
~
#38806
Olga greetings --> m38806~
0 g 100
~
   wait 5
%send% %actor% %self.name% asks you, with a husky voice, '&01Vell, do you want to leaf?&00'
~
#38807
Olga response --> m38806~
0 d 100
yes yes. Yes. Yes uh-huh sure sure.~
    if (%actor.class% == Thief)
        if (%actor.sex% == Male)  
          %send% %actor% Nodding swiftly, %self.name% grabs you by the neck and opens the tile, chucking you through.
          %echoaround% %actor% %self.name% nods swiftly, and grabs %actor.name% by the neck and opens the tile, chucking him through.
          wait 5
          %teleport% %actor.name% 38805
          %force% %actor% look
          wait 5
          %send% %actor% The tile closes gently below you.
        elseif (%actor.sex% == Neutral)
          %send% %actor% Smirking at your neutered state, %self.name% grabs you by the neck and opens the tile, chucking you through.
          %echoaround% %actor% %self.name% nods, smirks at %actor.name%, opens the tile, and gruffly chucks the neuter through.
          wait
          %teleport% %actor.name% 38805
          %force% %actor% look
          wait 5
          %send% %actor% The tile closes gently below you.
        else
          %send% %actor% Smiling pleasantly, %self.name% opens the tile and leads you through.
          %echoaround% %actor% %self.name% nods, smiles at %actor.name%, opens the tile, and leads her through gently.
          wait 5
          %teleport% %actor.name% 38805
          %force% %actor% look
          wait 5
          %send% %actor% The tile closes gently below you.
        end
    else
say &01You are not one of our secret guild. Begone!&00
%send% %actor.name% %self.name% throws you from the room so hard that at first you don't know where you are.
%teleport% %actor.name% 18022
%force% %actor% look
end
~
#38808
Olga negative reply --> m38808~
0 d 100
No No. Nope. Nope nope no. nope. no bah~
   if (%actor.class% == Thief)
%send% %actor% %self.name% frowns at you, and belts out, '&01Vell, leaf me alone, den. I am busy.&00'
      %send% %actor% %self.name% shoves you away from the stairs.
      %echoaround% %actor% %self.name% grabs %actor.name% by the neck, and forces %actor.name% away from the stairs.
%send% %actor% %self.name% grabs you and throws you from the room.
      %force% %actor% south
      %echo% Sniffing loudly, %self.name% returns, wipes her hands off, and stands at the ready.
   else
say &01You don't belong here! Guards!&00
kill %actor.name%
end
~
#38809
Thief guild escape chute trigger~
2 c 100
enter~
%send% %actor% &14As you hop into the escape chute, you start picking up speed and racing through the darkness.&00
wait 3
%send% %actor% &14The walls race by at an incredible speed until you see a light fast approaching.&00
wait 3
%send% %actor% &14You slide out of a hole in the wall and land on your feet only to look back and realize the hole is covered in illusion.&00
%teleport% %actor.name% 18022
%force% %actor% look
end
~
#38810
Trigger to get to Th guild questmaster~
0 d 100
"tirok nuthm"~
say It is obvious you have learned much in your time with us.
wait 40
say I don't know how you found out about our greatest secret, but perhaps it is time for you to learn the rest.
wait 40
say Inside rests one of the ancient ones. A being with great power that took our greatest sages to hold.
wait 40
say Even then I think he stays more of his own volition than any restraint we may put on him.
wait 40
say He seems to delight in testing our people with riddles and quests.
wait 40
say The weak are lost in their pursuits of easy money and power
wait 40
say But the truly great are often given valuable gifts.
wait 40
mload obj 38841
unlock bookcase
open bookcase
wait 35
%echo% %self.name% opens the hidden pasage behind her and bids you to enter.
end
~
#38820
Trigger so items can only be stolen as they are destroyed before death--> Attached to m40354~
0 l 35
~
        mjunk all
        say If I can't have this treasure, nobody can!
      cackle
    wait 500
~
#38821
Th guild quartermaster lets 45+ thieves know about the quest to start quest dagger series~
0 g 100
~
if %actor.class% == Thief && %actor.level% > 45
say Psssttttt! Over here! I have a job for you %actor.name%!
   halt
   end 
say You don't belong in here %actor.name%! Leave before I have you killed!
%send% %actor.name% %self.name% calls the guards and has you violently thrown from the guild!
   %teleport% %actor.name% 18022
   %force% %actor% look
~
$~
