#31138
mob_load~
1 c 100
*~
if (%cmd% == open || %cmd% == op)
     if (%self.name% /= %arg% && %arg.strlen% > 0)
        %echo% A sleepy vampire says, 'How dare you desturb my slumber!'
        %load% mob 21103
        %force% vampire kill %actor.name%
         %purge% %self%
      else
        return 0
        halt 
     end
   else
     return 0
   end
~
$~
