#!/usr/bin/perl
$| = 1;

# Note that because fork loses test count we do not use Test::More

use strict;

BEGIN {
  require Config;
  my $can_fork = $Config::Config{d_fork} ||
    (($^O eq 'MSWin32' || $^O eq 'NetWare') and
     $Config::Config{useithreads} and
     $Config::Config{ccflags} =~ /-DPERL_IMPLICIT_SYS/
    );
  if ( $can_fork && !(($^O eq 'MSWin32') && $Devel::Cover::VERSION) ) {
    print "1..8\n";
  } else {
    if ( ($^O eq 'MSWin32') && $Devel::Cover::VERSION ) {
        print "1..0 # Skip Devel::Cover coverage testing is incompatible with fork under 'MSWin32'\n";
    } else {
        print "1..0 # Skip No fork available\n";
    }
    exit;
  }
}

use File::Temp;

# OO interface

my $file = File::Temp->new();

myok( 1, -f $file->filename, "OO File exists" );

my $children = 2;
for my $i (1 .. $children) {
  my $pid = fork;
  die "Can't fork: $!" unless defined $pid;
  if ($pid) {
    # parent process
    next;
  } else {
    # in a child we can't keep the count properly so we do it manually
    # make sure that child 1 dies first
    my $time = ($i-1) * 3;
    print "# child $i sleeping for $time seconds\n";
    sleep($time);
    my $count = $i + 1;
    myok( $count, -f $file->filename(), "OO file present in child $i" );
    print "# child $i exiting\n";
    exit;
  }
}

while ($children) {
    wait;
    $children--;
}



myok( 4, -f $file->filename(), "OO File exists in parent" );

# non-OO interface

my ($fh, $filename) = File::Temp::tempfile( UNLINK => 1 );

myok( 5, -f $filename, "non-OO File exists" );

$children = 2;
for my $i (1 .. $children) {
  my $pid = fork;
  die "Can't fork: $!" unless defined $pid;
  if ($pid) {
    # parent process
    next;
  } else {
    my $time = ($i-1) * 3;
    print "# child $i sleeping for $time seconds\n";
    sleep($time);
    my $count = 5 + $i;
    myok( $count, -f $filename, "non-OO File present in child $i" );
    print "# child $i exiting\n";
    exit;
  }
}

while ($children) {
    wait;
    $children--;
}
myok(8, -f $filename, "non-OO File exists in parent" );


# Local ok sub handles explicit number
sub myok {
  my ($count, $test, $msg) = @_;

  if ($test) {
    print "ok $count - $msg\n";
  } else {
    print "not ok $count - $msg\n";
  }
  return $test;
}
