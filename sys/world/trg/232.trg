#23200
Test trigger~
2 g 100
~
%echo% Vnum %self.vnum%
~
#23201
Commando Instant Poison Off-balance trigger~
0 e 0
is off balance and vulnerable!~
eval success %random.3%
if %success% < 3
halt
end
wait 1
say Yaah hacha haa-ya!!
wait 2
poison %actor.name%
wait 1
say Hah ti-i!
~
#23280
Generic Maze Randomizer~
2 af 100
~
*For: Rooms  Type: Global and Zone Reset
*This script is designed for a 4 room maze
*It will randomly generate a maze given 4 room vnums
*whenever the zone resets.  
*%self.vnum% represents the room that the trigger is
*attached to. Be careful and make sure that there is
*always a path through the maze.  
*pick a number between 1 and 3
eval roomset %random.3%
 if %roomset% == 1
   * Purge all the exits first
   wdoor %self.vnum% west purge
   wdoor %self.vnum% east purge
   wdoor %self.vnum% north purge
   wdoor %self.vnum% south purge
   * create new exits
   wdoor %self.vnum% west room 23280^M
   wdoor %self.vnum% east room 23281^M
   wdoor %self.vnum% south room 23282^M
   wdoor %self.vnum% north room 23283^M
 else
 if %roomset% == 2
   * purge all the exits
   wdoor %self.vnum% west purge
   wdoor %self.vnum% east purge
   wdoor %self.vnum% north purge
   wdoor %self.vnum% south purge
   * create new ones
   wdoor %self.vnum% west room 23283^M
   wdoor %self.vnum% east room 23282^M
   wdoor %self.vnum% south room 23281^M
   wdoor %self.vnum% north room 23280^M
  else
   * purge all the exits
   wdoor %self.vnum% west purge
   wdoor %self.vnum% east purge
   wdoor %self.vnum% north purge
   wdoor %self.vnum% south purge
   * create new ones
   wdoor %self.vnum% west room 23283^M
   wdoor %self.vnum% east room 23281^M
   wdoor %self.vnum% south room 23280^M
   wdoor %self.vnum% north room 23282^M
   end
  end 
 
~
#23281
The Super Super Aggro script~
0 h 100
~
say You're beginning to bore me, %actor.name%
kill %actor.name%
~
#23298
Guard Assistance ~
0 l 25
~
eval roomid %self.room%
      eval roomname %roomid.name%
      shout Guards! Guards! To me! I am under attack at %roomname%!
      %load% mob 23299
      eval guardmob %roomid.people%
      %echo% A guard charges up the hallway.
      %force% %guardmob% rescue maud
      %force% %guardmob% assist maud
      %load% mob 23299
      eval guardmob %roomid.people%
      %echo% A guard charges up the hallway.
      %force% %guardmob% rescue maud
      %force% %guardmob% assist maud
      wait 400
~
#23299
Viewing room~
2 g 100
~
if %direction% == south
return 0
end
~
$~
