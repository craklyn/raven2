#!/usr/bin/perl
#--------------------------------------------------------------------------
# Heavily Modified by Lawrence Cecil for use with RavenMud 2.0
# 09/08/2000 Added Sendmail Functionality for mail on reboot/failure
# SMTP Modules credit to Mark Scattergood
#
# Tweaked, poked and twisted by Digger on and about 10/31/00
#
#--------------------------------------------------------------------------

use Carp;
use Cwd;
use Env;

#
my %mudParams =(
  main    => {
    port  => 6060,
    home  => "$ENV{HOME}/RavenMUD",
    flags => "-q",
    loop  => 1,
    sufx  => "Main"
  },

  builder => {
    port  => 6666,
    home  => "$ENV{HOME}/TestMUD",
    flags => "-q",
    loop  => 1,
    sufx  => "Builder"
  },

  test    => {
    port  => 5986,
    home  => "$ENV{HOME}/testport",
    flags => "-q",
    loop  => 1,
    sufx  => "Test"
  }

);


#--------------------------------------------------------------------------
#
# Runtime Var Setup
#
# .fastboot        sleep for only 5 seconds, instead of the default 20
# .killscript        script will exit, and mud will not reboot
# .pause        pause (sleep 1 minute intervals) until .pause it removed
#==========================Main Mud Loop===========================
#
if( !( $ARGV[0] =~ /^test$|^builder$|^main$/ ))
{
  print << "EOF";

  Illegal argument: @ARGV\n  Usage: startraven main|builder|test

    startraven main &    -  Starts Ravenmud on port $mainport
    startraven builder & -  Starts Ravenmud Builder port on $buildport
    startraven test &    -  Starts Ravenmud Test port on $testport

EOF
  exit;
}

my $MODE  = $ARGV[0];
my $HOME  = $mudParams{$MODE}{home};
my $PORT  = $mudParams{$MODE}{port};
my $FLAGS = $mudParams{$MODE}{flags} . " -d $HOME/sys";
my $SUFX  = $mudParams{$MODE}{sufx};

my $DATE  = `date`;

# Check for one already running.
#
$pid = `ps -x | grep RavenMUD-$SUFX | grep -v $$ | grep -v grep`;
$pid =~ s/^.*?\s+(\d+)\s+.*$/$1/gs;
if( $pid > 0 )
{
  print "\nRavenMUD $MODE is already running with Process ID ".$pid."\n\n";
  exit 0;
}

# Set the process name and move to the root directory.
#
$0 = "RavenMUD-$SUFX";
chdir $HOME;


# Start the mainloop
#
while(1)
{
  # Open SYSLOG and dup STDERR into SYSLOG
  #
  open( SYSLOG, ">> $HOME/syslog" );
  open( STDERR, ">& SYSLOG" );

  print SYSLOG "$DATE Script $0 Running ";
  open( SERVER, "$HOME/bin/moon $FLAGS $PORT |" );

  # Run the server and log the output.
  #
  while(<SERVER>) { print SYSLOG; }

  # do any pending eq refreshes
  updateEq();

  # The server went down - grep the good stuff and rotate the logs.
  #
  maintainLogs( $HOME );

  # Should we stay dead?
  #
  if( -r '$HOME/.killscript' )
  {
    unlink '$HOME/.killscript';
    open (SYSLOG, '>> $HOME/log/syslog.1');
    print SYSLOG "autoscript killed $DATE";
    exit;
  }

  # Or just play dead?
  #
  while( -r '$HOME/.pause' )
  {
    sleep 60;
  }

  # Or reboot as soon as possible?
  #
  if( -r '$HOME/.fastboot' )
  {
    unlink '$HOME/.fastboot';
    sleep 5;
  }

  else { sleep 20 }

}

#--------------------------------------------------------------------------
#
# update pending eq refreshes
sub updateEq {
    if (open UPDATES, "$HOME/sys/etc/updates") {
        my $vnums = "";
        my $count = 0;

        while (my $vnum = <UPDATES>) {
          chomp($vnum);
          $vnums .= "$vnum ";
          $count++;
          if ($count == 19) {
            print SYSLOG "Updating $vnums\n";
            system("$HOME/bin/equpdate $vnums");
            $vnums = ""; $count = 0;
          }
        }
        if ($count > 0) {
            print SYSLOG "Updating $vnums\n";
            system("$HOME/bin/equpdate $vnums")
        }
        close UPDATES;
        unlink("$HOME/sys/etc/updates");
    }
}


#--------------------------------------------------------------------------
#
# Log file information. This stuff contains the name of the file to write
# to and the text string to grep for.
#
sub maintainLogs
{
  my $LOGROOT = shift;
  my $LOGDATE = `date`;
  my $LOGENT  = "";
  my @LOGTXT  = ();

  my %logEntries =(
    badpws       => "Bad PW",
    crashcmds    => "LAST COMMAND",
    deaths        => "killed",
    delete       => "self-delete",
    dtraps       => "death trap",
    errors       => "SYSERR",
    godcmds      => "(GC)",
    levels       => "advanced",
    metaphys     => "META",
    newplayers   => "new player",
    objecterrors => "OBJ",
    olcerrors    => "OLCERR",
    olcmods      => "OLC",
    prayers      => "Prayer",
    reboots      => "Running",
    rentgone     => "equipment lost",
    transcend     => "(TRANSCEND)",
    trigger      => "TRIGERR",
    usage        => "usage",
  );

  # First, load up the syslog into an array.
  #
  open SYSLOG, "<$LOGROOT/syslog" or die "Cannot open $LOGROOT/syslog";
  while(<SYSLOG>)
  {
    push @LOGTXT, $_;
  }
  close SYSLOG;

  # Now rip through it looking for things of interest.
  #
  foreach $LOGENT ( keys %logEntries )
  {
    my $LOGNAME = "$LOGROOT/log/$LOGENT";
    open LOGFILE, ">>$LOGNAME" or die "Cannot open $LOGNAME";
    print LOGFILE "--------------------------------------------- $LOGDATE";
    foreach $LINE (@LOGTXT)
    {
      if( $LINE =~ /$logEntries{$LOGENT}/ )
      {
        print LOGFILE $LINE;
      }
    }

    close LOGFILE;
  }

  unlink ('log/syslog.6') or die "Cannot rotate $LOGROOT/log/syslog.6";
  rename ('log/syslog.5', 'log/syslog.6');
  rename ('log/syslog.4', 'log/syslog.5');
  rename ('log/syslog.3', 'log/syslog.4');
  rename ('log/syslog.2', 'log/syslog.3');
  rename ('log/syslog.1', 'log/syslog.2');
  rename ('syslog'      , 'log/syslog.1');

}

#================================================================================================================================================================
