#!perl -w
# Test overloading

use Test::More tests => 19;
use strict;

BEGIN {use_ok( "File::Temp" ); }

{
  my $fh = File::Temp->new();
  isa_ok ($fh, 'File::Temp');

  ok( "$fh" ne "foo", "compare stringified object with string");
  ok( $fh ne "foo", "compare object with string");
  ok( $fh eq $fh, "compare eq with self");

  ok( $fh != 0, "compare != 0");
  ok( $fh == $fh, "compare == with self");
  ok( $fh != \*STDOUT, "compare != \*STDOUT");

  {
    my $num = $fh+0;
    like ($num, qr/^\d+$/, '+0 is a number');
  }
  {
    my $str = "$fh";
    unlike ($str, qr/^\d+$/, '"" is not a number');
  }
}

{
  my $fh = File::Temp->newdir();
  isa_ok ($fh, 'File::Temp::Dir');

  ok( "$fh" ne "foo", "compare stringified object with string");
  ok( $fh ne "foo", "compare object with string");
  ok( $fh eq $fh, "compare eq with self");

  ok( $fh != 0, "compare != 0");
  ok( $fh == $fh, "compare == with self");
  ok( $fh != \*STDOUT, "compare != \*STDOUT");

  {
    my $num = $fh+0;
    like ($num, qr/^\d+$/, '+0 is a number');
  }
  {
    my $str = "$fh";
    unlike ($str, qr/^\d+$/, '"" is not a number');
  }
}
