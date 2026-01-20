#21256
King Greybeard's reward trigger for emerald~
0 j 100
emerald~
   if %object.vnum% == 21314
   wait 1 s
   gasp
   wait 2 s
   say Thank you! Oh, thank you so much.
   wait 2 s
   say With this gem my people are saved.
   wait 2 s
   say This is incredible... I have no way of expressing my joy.
   wait 2 s
   say This is all the money our treasury has. I hope this helps you as you have so kindly helped our city.
   %load% obj 21281
   give reward %actor.name%
   wait 1 s
   say Thank you again %actor.name%, we are forever in your debt.
   wait 1 s
   bow
   mjunk all
   else
   wait 1 s
   say This item will not help my people. Go peddle your junk elsewhere!
   wait 1 s
   drop all
   endif
~
$~
