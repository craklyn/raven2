#!/usr/bin/perl
#
# If the modification time of the filename referenced by arg 1 exceeds
# the number of seconds specified by arg 2 then return the number of
# seconds since the last mod time, otherwise return nothing.
#
$fage = shift(@ARGV);
$fsec = shift(@ARGV);

($dev,$ino,$mode,$nlink,$uid,$gid,$rdev,$size,
  $atime,$mtime,$ctime,$blksize,$blocks) = stat $fage;

$now = time;
$modified = $now-$mtime;

if( $modified > $fsec )
{
  print $modified;
}
