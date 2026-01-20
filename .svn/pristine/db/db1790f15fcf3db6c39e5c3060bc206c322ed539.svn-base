#!/usr/bin/perl

# This is going to be needed once, but it'll make life a HELL of alot
# easier. This script will go into an existing RavenMUD C file and tweak
# the include statements to conform to the new subdir approach. This new
# approach, BTW, is the CORRECT approach.
#
@MUDINCS = qw (
  actions/auction.h actions/clan.h actions/combat.h actions/commact.h
  actions/command_list.h actions/disasters.h actions/fight.h
  actions/informative.h actions/insert_items.h actions/interpreter.h
  actions/movement.h actions/objact.h actions/offensive.h actions/other.h
  actions/outlaw.h actions/suffer.h actions/wizard.h

  general/class.h general/color.h general/comm.h general/db.h
  general/handler.h general/lulib.h general/modify.h general/objsave.h
  general/rmath.h general/structs.h

  magic/aggressive.h magic/backstab.h magic/brain.h magic/gut.h
  magic/knock.h magic/learn.h magic/magic.h magic/missile.h magic/rescue.h
  magic/skills.h magic/spells.h magic/stun.h magic/trap.h

  olc/copy.h olc/medit.h olc/oedit.h olc/olc.h olc/redit.h olc/sedit.h
  olc/zedit.h

  specials/assign.h specials/beholder.h specials/bloodbowl.h specials/boards.h
  specials/castle.h specials/combspec.h specials/contractkill.h
  specials/dice.h specials/guard.h specials/healer.h specials/house.h
  specials/lands_of_chaos.h specials/legend.h specials/mail.h
  specials/metaphys.h specials/mobact.h specials/perceptory.h
  specials/portal.h specials/reward.h specials/seek.h specials/shop.h
  specials/special.h specials/superAggr.h specials/ten_trials.h
  specials/torment.h specials/unicorn.h

  util/logging.h util/utils.h util/weather.h
);

%HCROSSREF = ();

# Chunk through and build a hash based on the header filename. This could
# be done in a more automated fashion, but using the static qw approach
# from above will be sufficient since we will only have to do it once.
#
foreach $MUDH ( @MUDINCS )
{
  my ($HSDIR, $HFILE) = $MUDH =~ /(\S+)\/(\S+)/;
  $HCROSSREF{$HFILE}  = $MUDH;
}

# Read through STDIN looking for mud specific include statement and replace
# them as necessary.
#
while(<>)
{
  if( /^\#include \"(\S+)\"/ )
  {
    if( defined( $HCROSSREF{$1} ))
    {
      print "#include \"$HCROSSREF{$1}\"\n";
    }
    else
    {
      print;
    }
  }
  else
  {
    print;
  }
}

