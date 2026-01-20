#21138
mob_load~
1 c 4
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
#21140
alcove A entrance~
1 c 4
*~
if (%cmd% == move || %cmd% == mov)
           if (%actor.is_npc% != 0 || %actor.level% > 50)
                halt
   elseif (%actor.str% >= 24)
              if (%self.name% /= %arg% && %arg.strlen% > 0)
               %echo% A coffin slides out of the way, exposing an alcove in the wall.
               %door% 21138 north room 21140
               wait 150
               %door% 21138 north purge
                else
                return 0
                end
            else
              return 0
              end
          else
            return 0
            end
~
#21152
alcove B entrance~
1 c 4
*~
if (%cmd% == move || %cmd% == mov)
           if (%actor.is_npc% != 0 || %actor.level% > 50)
                halt
   elseif (%actor.str% >= 24)
              if (%self.name% /= %arg% && %arg.strlen% > 0)
               %echo% A coffin slides out of the way, exposing an alcove in the wall.
               %door% 21151 south room 21152
               wait 150
               %door% 21151 north purge
                else
                return 0
                end
            else
              return 0
              end
          else
            return 0
            end
~
$~
