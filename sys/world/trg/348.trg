#34800
Anti Steal on Witch -->m34803~
0 c 100
*~
if %cmd.mudcommand% == steal || %cmd.mudcommand% == mug
%send% %actor.name% The old witch snaps her head in your direction and gives you the evil &08eye&00!
%echoaround% %actor.name% The witch gives %actor.name% the evil &08eye&00.
   %damage% %actor.name% 100
else
   return 0
end
~
$~
