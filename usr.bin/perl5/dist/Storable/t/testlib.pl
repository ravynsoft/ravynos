#!perl -w
use strict;

our $file = "storable-testfile.$$";
die "Temporary file '$file' already exists" if -e $file;

END { while (-f $file) {unlink $file or die "Can't unlink '$file': $!" }}

use Storable qw (store retrieve freeze thaw nstore nfreeze);

sub slurp {
  my $file = shift;
  local (*FH, $/);
  open FH, "<", $file or die "Can't open '$file': $!";
  binmode FH;
  my $contents = <FH>;
  die "Can't read $file: $!" unless defined $contents;
  return $contents;
}

sub store_and_retrieve {
  my $data = shift;
  unlink $file or die "Can't unlink '$file': $!";
  local *FH;
  open FH, ">", $file or die "Can't open '$file': $!";
  binmode FH;
  print FH $data or die "Can't print to '$file': $!";
  close FH or die "Can't close '$file': $!";

  return eval {retrieve $file};
}

sub freeze_and_thaw {
  my $data = shift;
  return eval {thaw $data};
}

1;
