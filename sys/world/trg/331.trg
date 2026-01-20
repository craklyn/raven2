#33100
maze trigger~
2 f 100
~
* establish the maze number of this room
eval num %self.vnum% - 33179
* pick random rooms for the exits
eval exit1 33179 + %random.num%
eval exit2 33179 + %random.num%
eval exit3 33179 + %random.num%
eval exit4 33179 + %random.num%
eval exit5 33179 + %random.num%
eval exit6 33179 + %random.num% 
* set the door for a random to %self.vnum% + 1
eval exit%random.6% %self.vnum% + 1
* place exits
%door% %self.vnum% north room %exit1%
%door% %self.vnum% south room %exit2%
%door% %self.vnum% east  room %exit3%
%door% %self.vnum% west  room %exit4%
%door% %self.vnum% up    room %exit5%
%door% %self.vnum% down  room %exit6%
%echo% The room shifts and blurs!
~
#33101
Clock ticker - carried~
1 f 100
~
eval wearer %self.carried_by%
if %wearer%
  %send% %wearer% The clock chimes the hour.
end 
otimer 1
~
#33102
Clock ticker - worn~
1 f 100
~
   eval wearer %self.worn_by%
   if %wearer%
     %send% %wearer% The clock chimes the hour.
   end 
   otimer 1
~
#33103
Clock ticker - room~
1 f 100
~
%echo% The clock chimes the hour.
otimer 1
~
#33104
Agnos's Brewing~
0 g 80
~
wait 1 s
if %random.4% < 4
  %echo% The potion in Agnos's hands explodes violently!
else
  %echo% Agnos creates a potion!
end 
~
#33190
mistletoe~
1 b 100
*~
eval people %people.<18102>% 
%echo% there are %people% in the room.
~
$~
