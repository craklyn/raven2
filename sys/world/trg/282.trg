#28200
Mckamey spell cast~
0 k 100
~
dg_cast 'cleanse' self
break
~
#28201
Mckamey 80%~
0 l 80
~
eval %self.id%
%echo% %self.name% says, '&07&25RAISE FROM THE DEAD MY SON, AND SLAUGHTER THOSE BEFORE YOU!&00
%load% mob 28201
detach 28201 %self.id%
~
#28202
Mckamey 60%~
0 l 60
~
eval %self.id%
%echo% %self.name% says, '&07&25RAISE FROM THE DEAD MY DAUGHTER, AND AIDE ME IN BATTLE!&00
%load% mob 28202
detach 28202 %self.id%
~
#28203
Mckamey 40%~
0 l 40
~
eval %self.id%
%echo% %self.name% says, '&07&25RAISE FROM THE AFTERLIFE MY LOVE, AND TORTURE THESE SOULS BY MY SIDE!&00
%load% mob 28203
detach 28203 %self.id%
~
#28204
mckamey 20~
0 l 20
~
eval %self.id%
%echo% %self.name% screams, '&22&25FROM THE DEPTHS OF HELL, RISE MY FAMILY, AND SEEK OUR REVENGE!&00
%load% mob 28203
%load% mob 28202
%load% mob 28201
detach 28204 %self.id%
~
$~
