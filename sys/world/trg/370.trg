#37001
Injured Warrior~
0 h 100
~
if (%direction% == south)
  wait 1 sec
  shiver
  wait 3 sec
  cough
  wait 4 sec
  %echo% The warrior doesn't look so well.
end
~
#37002
Door Re-Lock~
2 g 100
~
wait 1 sec
%echo% The door shuts behind you.
wait 1 sec
%echo% *CLICK*
~
#37003
DELETEME~
0 g 100
DELETEME~
DELETEME
~
#37007
Alarms from the West~
2 g 100
~
if (%direction% == west) 
  if %370hasranlight1%
    wait 5 sec
    %echo% The alarms have been tripped.
  else
    * Borrowed and modified from Rumble's switch example
    wait 1 second
    switch %random.10%
      case 1
        %send% %actor% You stumble as the thin wire trips you.
        %echoaround% %actor% %actor.name% falls to the ground after being tripped by a very thin wire.
      break
      case 2
        %send% %actor% A small tile in the floor sinks as you step on it.
        %echoaround% %actor% A small tile in the floor sinks as %actor.name% steps on it.
      break
      case 3
        %send% %actor% A small camera has recorded your entry in the room.
        %echoaround% %actor% As %actor.name% enters the room, you notice a small camera tracking the movement.
      break
      case 4
        %send% %actor% A small dart hits you in the neck.
        %echoaround% %actor% A small dark comes out of the wall and hits %actor.name% in the neck.
      break
      case 5
        %send% %actor% As you enter the room you accidentally step on a small button in the floor.
        %echoaround% %actor% %actor.name% steps on a small button on the floor.
      break
      case 6
        %send% %actor% You feel as if all eyes are on you.
        %echoaround% %actor% All eyes are on %actor.name% as they enter the room.
      break
      case 7
        %send% %actor% As you enter the room a small machine notices your entrance and flees to the east.
        %echoaround% %actor% As %actor.name% enters the room a small machine notices and flees in the darkness.
      break
      case 8
        %send% %actor% You hear a loud CRUNCH as you step on broken glass.
        %echoaround% %actor% %actor.name% looks very foolish as they make entirely too much noise.
      break
      case 9
        %send% %actor% You realize your mistake as you enter the room.
        %echoaround% %actor% %actor.name% has made a horrible mistake and alerted all to your presence.
      break
      case 10
        %send% %actor% You stumble into the room and cross paths with infrared beams.
        %echoaround% %actor% %actor.name% stumbles into the room and breaks the solid lines of infrared beams.
      break
      default
        %send% %actor% You have activated the silent alarms.
        %echoaround% %actor% %actor.name% has activated the silent alarm.
      break
    done
    wait 1 sec
    %echo% A small red light on the wall begins to blink.
    %load% obj 37012
    wait 5 sec
    %echo% The alarms have been tripped.
    set 370hasranlight1 1
    global 370hasranlight1
  end
end
~
#37011
DELETEME~
2 c 100
p~
DELETEME
~
#37012
Alarms from the East~
2 g 100
None~
if (%direction% == east) 
  if %370hasranlight1%
    wait 5 sec
    %echo% The alarms have been tripped.
  else
    * Borrowed and modified from Rumble's switch example
    wait 1 second
    switch %random.10%
      case 1
        %send% %actor% You stumble as the thin wire trips you.
        %echoaround% %actor% %actor.name% falls to the ground after being tripped by a very thin wire.
      break
      case 2
        %send% %actor% A small tile in the floor sinks as you step on it.
        %echoaround% %actor% A small tile in the floor sinks as %actor.name% steps on it.
      break
      case 3
        %send% %actor% A small camera has recorded your entry in the room.
        %echoaround% %actor% As %actor.name% enters the room, you notice a small camera tracking the movement.
      break
      case 4
        %send% %actor% A small dart hits you in the neck.
        %echoaround% %actor% A small dark comes out of the wall and hits %actor.name% in the neck.
      break
      case 5
        %send% %actor% As you enter the room you accidentally step on a small button in the floor.
        %echoaround% %actor% %actor.name% steps on a small button on the floor.
      break
      case 6
        %send% %actor% You feel as if all eyes are on you.
        %echoaround% %actor% All eyes are on %actor.name% as they enter the room.
      break
      case 7
        %send% %actor% As you enter the room a small machine notices your entrance and flees to the east.
        %echoaround% %actor% As %actor.name% enters the room a small machine notices and flees in the darkness.
      break
      case 8
        %send% %actor% You hear a loud CRUNCH as you step on broken glass.
        %echoaround% %actor% %actor.name% looks very foolish as they make entirely too much noise.
      break
      case 9
        %send% %actor% You realize your mistake as you enter the room.
        %echoaround% %actor% %actor.name% has made a horrible mistake and alerted all to your presence.
      break
      case 10
        %send% %actor% You stumble into the room and cross paths with infrared beams.
        %echoaround% %actor% %actor.name% stumbles into the room and breaks the solid lines of infrared beams.
      break
      default
        %send% %actor% You have activated the silent alarms.
        %echoaround% %actor% %actor.name% has activated the silent alarm.
      break
    done
    wait 1 sec
    %echo% A small red light on the wall begins to blink.
    %load% obj 37012
    wait 5 sec
    %echo% The alarms have been tripped.
    set 370hasranlight1 1
    global 370hasranlight1
  end
end
detach 37012 %self.id%
~
#37013
Blinker Mobs~
1 c 100
l~
if %cmd.mudcommand% == look && 'light /= '%arg% && %arg.strlen% > 0
  %send% %actor% It is nothing more than a light. You start to feel uneasy however, when it stops blinking.
  %echoaround% %actor% As %actor.name% looks at the blinking light, the room starts to shake.
wait 1 sec
  %echo% The vegetation begins to stir.
wait 1 sec
  %load% mob 37012
  %echo% The room becomes vibrant with movement.
  %load% mob 37012
  %purge% blinking
else
  return 0
end
~
#37014
DELETEME~
2 c 100
p~
DELETEME
~
#37015
Zone Reset - Remove Var~
2 f 100
~
if %370hasranlight1%
  unset 370hasranlight1
end
~
#37016
Zone Reset - Remove Var~
2 f 100
~
if %hasran%
  unset hasran
end
~
#37017
DELETEME~
2 g 100
~
DELETEME
~
#37018
DELETEME~
2 g 100
~
DELETEME
~
#37019
Statues~
1 c 100
look~
if %arg% == statues
  switch %random.05%
    case 1
      %send% %actor% The statues glare at you.
      %echoaround% %actor% As the statues glare at %actor.name% you start to feel uneasy.
    break
    case 2
      %send% %actor% The statues whisper to you, 'Fear not, for we will guide you.'
      %echoaround% %actor% The statues whisper something to %actor.name%.
    break
    case 3
      %send% %actor% They seem to be mocking you.
      %echoaround% %actor% The statues laugh and bellow, 'What a FOOL %actor.name% is. Shall we crush them?'
    break
    case 4
      %send% %actor% The statues look at you and laugh.
      %echoaround% %actor% The statues smirk and say, 'Perhaps you should let %actor.name% fight us alone?'
    break
    case 5
      %echo% The statues snicker and say, "We will not fight you unless provoked, %actor.name%."
    break
    default
      %echo% The statues snicker and say, 'We will not fight unless provoked, %actor.name%'.
    break
  done
end
~
#37020
Statue Option~
1 c 100
slap~
if %arg% == statues
  %echo% The statues look at each other and grin.
  wait 1 sec
  %load% mob 37019
  %load% mob 37019
  %purge% statues
else
  return 0
end
~
#37033
DELETEME~
1 c 100
l~
DELETEME
~
#37034
DELETEME~
1 c 100
e~
DELETEME
~
#37035
Machines~
1 c 100
look~
if %arg% == machine
  switch %random.05%
    case 1
      %send% %actor% As you glance at these machines you realize one of them has a small button.
      %echoaround% %actor% %actor.name% looks curiously at a small button.
    break
    case 2
      %send% %actor% There seems to be a small button at the base of one of these machines.
      %echoaround% %actor% %actor.name% points out a small button on the floor.
    break
    case 3
      %send% %actor% The machines whisper to you, 'Fear not, we shall protect you.'
      %echoaround% %actor% The machines whisper something to %actor.name%.
    break
    case 4
      %send% %actor% The machines glare at you.
      %echoaround% %actor% The machines seem to be glaring at %actor.name%.
    break
    case 5
      %echo% The machines laugh heartily.
      wait 1 sec
      %echo% Keep moving you mindless twits.
    break
    default
      %echo% The machines snicker and say, 'Go ahead fools, push the button!'
    break
  done
end
~
#37036
Machine Option~
1 c 100
push~
if %arg% == button
  %send% %actor% You push a small button and nothing happens.
  %echoaround% %actor% %actor.name% pushes a small button.
  wait 1 sec
  %echo% A cool breeze blows through the room.
  %load% mob 37035
  %load% mob 37035
  %purge% machines
else
  return 0
end
~
#37049
DELETEME~
1 c 100
l~
DELETEME
~
#37050
DELETEME~
1 c 100
e~
DELETEME
~
#37070
Entrance to the Suite~
2 c 100
k~
if %cmd.mudcommand% == knock && 'twice /= '%arg% && %arg.strlen% > 0
  %send% %actor% You knock on the door twice.
  %send% %actor% Before you know it, you are in the President's suite!
  4:   %teleport% %actor% 37816
  %force% %actor% look
else
  return 0
end
~
#37071
house guard test~
2 c 100
w~
if %actor.name% != ish
  %send% %actor% Nice try, but this isn't your house!
  %damage% %actor% 100
  %force% %actor% dismount
  %teleport% %actor% 18001
  %echo% %actor.name% suddenly falls into a hole!
  %force %actor% look
else
  return 0
end
~
#37072
Guard Machine~
0 h 100
~
if %direction% == west
  wait 1 sec
  %echo% The Gate-Keeper tries to communicate with you at first with a series of beeps.
  wait 3 sec
  %echo% The Gate-Keeper tries to communicate with you by using a series of changing languages.
  wait 5 sec
  %echo% The Gate-Keeper glares at you before losing interest.
else
  if %direction% == east
    wait 1 sec
    %echo% The Gate-Keeper tries to communicate with you at first with a series of beeps.
    wait 3 sec
    %echo% The Gate-Keeper tries to communicate with you using a series of changing languages.
    wait 5 sec
    %echo% The Gate-Keeper glares at you before losing interest.
  else
  end
end
~
#37090
Thief Only~
2 d 100
"I'm a thief!"~
if %actor.class% != Thief
  wait 1 sec
  %send% %actor% A thief spits in your face and says, 'Yeah, and I'm your mum!'
  %echoaround% %actor% A thief spits in the face of %actor.name%.
  halt
end
wait 1 sec
%send% %actor% A thief winks and says, 'Please, into the &07shadows&00.'
wait 1 sec
%send% %actor% A thief says, 'Hurry now!'
%echoaround% %actor% A thief winks at %actor.name%.
wait 1 sec
%teleport% %actor% 18001
%force% %actor% look
%echo% A thief nods as %actor.name% steps into the &07shadows&00.
~
#37091
Non-Thief Aggro~
1 cfg 0
hehehehe~
if %actor.level% > 50
  halt
end
if %actor.class% != Thief
  %teleport% %actor% 18001
  %force% %actor% look
  %echo% &08%actor.name% screams in pain as they are molecularly dissolved.&00 
else
  return 0
end
~
$~
