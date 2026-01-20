#12000
master feed~
0 h 100
~
if (%actor.is_npc% != 0 || %actor.level% > 50)
      halt
   else
      say How dare you enter my sanctum, %actor.name%!  For this, you shall die!
         kill %actor.name%
         feed %actor.name%
         end
~
#12001
glass drop-get~
1 gh 100
*~
if (%actor.is_npc% != 0 || %actor.level% > 50 || %actor.hitp% < 50)
      halt
   else
      %send% %actor% You cut your hand on the glass, splurting blood everywhere.
      %echoaround% %actor% %actor.name% cuts his hand on a piece of glass, splurting blood everywhere.
      %damage% %actor% 20
   end
~
#12002
guard~
0 l 10
~
mecho The Anointed One shouts, 'Legions of the night!  Quick!  To my aid!' 
mecho A guardian rushes to his charges aid!
mload mob 12015
mforce vampguard rescue anointed
mecho A guardian rushes to his charges aid!
mload mob 12015
mforce vampguard assist anointed
mecho A guardian rushes to his charges aid!
mload mob 12015
mforce vampguard rescue anointed
wait 400
~
#12003
coffin mob load~
1 c 4
*~
if (%cmd% == open || %cmd% == op)
     if (%self.name% /= %arg% && %arg.strlen% > 0)
        %echo% A sleepy vampire says, '&08How dare you desturb my slumber!&00'
        %load% mob 12018
        %force% sleepy kill %actor.name%
         %purge% %self%
      else
        return 0
        halt 
     end
   else
     return 0
   end
~
#12004
bats~
0 f 100
~
mecho &03a colony of bats swarm into the room.&00
%load% mob 12016
%load% mob 12016
%load% mob 12016
%load% mob 12016
%load% mob 12016
%load% mob 12016
~
#12005
glass give~
1 i 100
*~
if (%actor.hitp% < 50 || %victim.hitp% < 50)
      halt
   else
if (%actor.is_npc% != 0 || %actor.level% > 50)
        %send% %victim% You cut your hand on the glass, splurting blood everywhere.
        %echoaround% %victim% %victim.name% cuts his hand on a piece of glass, splurting blood everywhere.
        %damage% %victim% 20
elseif (%victim.is_npc% != 0 || %victim.level% > 50)
        %send% %actor% You cut your hand on the glass, splurting blood everywhere.
        %echoaround% %actor% %actor.name% cuts his hand on a piece of glass, splurting blood everywhere.
        %damage% %actor% 20
else
        %send% %actor% You cut your hand on the glass, splurting blood everywhere.
        %send% %victim% You cut your hand on the glass, splurting blood everywhere.
        %echoaround% %victim% %victim.name% cuts his hand on a piece of glass, splurting blood everywhere.
        %echoaround% %actor% %actor.name% cuts his hand on a piece of glass, splurting blood everywhere.
        %damage% %actor% 20
        %damage% %victim% 20
   end
~
#12006
stalagtite~
1 c 4
*~
if (%cmd% == pull || %cmd% == pul)
   if (%self.name% /= %arg% && %arg.strlen% > 0)
      if (%actor.is_npc% != 0 || %actor.level% > 50)
          halt
      elseif (%actor.hitp% < 100)
  %echo% You here a grinding screech emit from the walls, but nothing seems to happen.
      elseif (%actor.dex% >= 21)
          %echo% a loud grumbling noise emits from the walls as you think you hear heavy gears begin to turn.
          %send% %actor% You nimbly avoid a falling stalactite.
          %send% %actor% A secret panel slides open, exposing an exit to the east.
          %door% 12088 east room 12098
          wait 50
          %door% 12088 east purge
          %echo% The wall slides back into place covering the passage.
      else
          %echo% a loud grumbling noise emits from the walls as you think you hear heavy gears begin to turn.
          %send% %actor% Noticing a large stalactite begin to fall, you nimbly jump back, causing it to merely graze your leg.
          %damage% %actor% 100
          %force% %actor% north   
      end
    else
       return 0
       end
else
  return 0
  end
~
#12007
secret panel in study to east~
1 c 4
*~
if (%cmd% == pull || %cmd% == pul)
        if (%self.name% /= %arg% && %arg.strlen% > 0)
         %echo% A secret panel slides open, exposing an exit to the east.
         %door% 12029 east room 12030
 %door% 12030 west room 12029
         wait 80
         %door% 12029 east purge
 %door% 12030 west purge
         %echo% The wall slides back into place covering the passage.
        else
         return 0
         end
    else
      return 0
      end
~
#12008
secret panel into study~
1 c 4
*~
if (%cmd% == pull || %cmd% == pul)
        if (%self.name% /= %arg% && %arg.strlen% > 0)
         %echo% A secret panel slides open, exposing an exit to the west.
         %door% 12030 west room 12029
 %door% 12029 east room 12030
         wait 80
         %door% 12030 west purge
 %door% 12029 east purge
         %echo% The wall slides back into place covering the passage.
        else
         return 0
         end
    else
      return 0
      end
~
$~
