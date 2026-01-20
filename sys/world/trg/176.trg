#17600
exp giver~
0 f 100
~
   eval master %actor% 
   eval more 1
   while %more%
     if %master.master% && (%master.is_npc% || %master.is_grouped%)
       eval master %master.master%
     else
       eval more 0
     end
   done
   
   if %master.is_npc% == 0 && %master.room% == %actor.room% && %master.is_grouped%
     %send% %master% You actually really receive your share of experience this time -- 1000000 points.
     nop %actor.exp(1000000)%
   end
   
   eval follower 0
   meta ch master.follower(%follower%)
   while %ch%
     if %ch.is_npc% == 0 && %ch.room% == %actor.room% && %ch.is_grouped%
       %send% %ch% You actually really receive your share of experience this time -- 1000000 points.
       nop %actor.exp(1000000)%
     end
     eval follower %follower% + 1
     meta ch master.follower(%follower%)
   done
~
$~
