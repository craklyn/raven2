#4806
underdarkpw~
2 d 1
xraax ngitz~
if %speech% == xraax
wdoor 4750 north flags ad
wdoor 4750 north key 4620
wdoor 4750 north name jade
wdoor 4750 north room 4751
wdoor 4751 south flags ad
wdoor 4751 south key 4620
wdoor 4751 south name jade
wdoor 4751 south room 4750
%echo% The jade door silently swings open.
elseif %speech% == ngitz
wdoor 4750 north flags abcd
wdoor 4750 north key 4620
wdoor 4750 north name jade
wdoor 4750 north room 4751
wdoor 4751 south flags abcd
wdoor 4751 south key 4620
wdoor 4751 south name jade
wdoor 4751 south room 4750
%echo% The jade door swings shut, and locks.
end
halt
break
~
$~
