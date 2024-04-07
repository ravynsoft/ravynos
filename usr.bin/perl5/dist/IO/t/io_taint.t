#!./perl -T

use Config;

BEGIN {
    if ($ENV{PERL_CORE}
        and $Config{'extensions'} !~ /\bIO\b/ && $^O ne 'VMS'
        or not ${^TAINT}) # not ${^TAINT} => perl without taint support
    {
	print "1..0\n";
	exit 0;
    }
}

use strict;
if ($ENV{PERL_CORE}) {
  require("../../t/test.pl");
}
else {
  require("./t/test.pl");
}
plan(tests => 5);

END { unlink "./__taint__$$" }

use IO::File;
my $x = IO::File->new( "> ./__taint__$$" ) || die("Cannot open ./__taint__$$\n");
print $x "$$\n";
$x->close;

$x = IO::File->new( "< ./__taint__$$" ) || die("Cannot open ./__taint__$$\n");
chop(my $unsafe = <$x>);
eval { kill 0 * $unsafe };
SKIP: {
  skip($^O) if $^O eq 'MSWin32' or $^O eq 'NetWare';
  like($@, qr/^Insecure/);
}
$x->close;

# We could have just done a seek on $x, but technically we haven't tested
# seek yet...
$x = IO::File->new( "< ./__taint__$$" ) || die("Cannot open ./__taint__$$\n");
$x->untaint;
ok(!$?); # Calling the method worked
chop($unsafe = <$x>);
eval { kill 0 * $unsafe };
unlike($@,qr/^Insecure/);
$x->close;

TODO: {
  todo_skip("Known bug in 5.10.0",2) if $] >= 5.010 and $] < 5.010_001;

  # this will segfault if it fails

  sub PVBM () { 'foo' }
  { my $dummy = index 'foo', PVBM }

  eval { IO::Handle::untaint(PVBM) };
  pass();

  eval { IO::Handle::untaint(\PVBM) };
  pass();
}

exit 0;
