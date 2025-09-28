#!./perl

BEGIN {
  chdir 't' if -d 't';
  require './test.pl';
  set_up_inc('../lib');
}

plan tests => 45;

open(I, 'op/sysio.t') || die "sysio.t: cannot find myself: $!";
binmode I;

$reopen = ($^O eq 'VMS' ||
           $^O eq 'os2' ||
           $^O eq 'MSWin32');

$x = 'abc';

# should not be able to do negative lengths
eval { sysread(I, $x, -1) };
like($@, qr/^Negative length /);

# $x should be intact
is($x, 'abc');

# should not be able to read before the buffer
eval { sysread(I, $x, 1, -4) };
like($@, qr/^Offset outside string /);

# $x should be intact
is($x, 'abc');

$a ='0123456789';

# default offset 0
is(sysread(I, $a, 3), 3);

# $a should be as follows
is($a, '#!.');

# reading past the buffer should zero pad
is(sysread(I, $a, 2, 5), 2);

# the zero pad should be seen now
is($a, "#!.\0\0/p");

# try changing the last two characters of $a
is(sysread(I, $a, 3, -2), 3);

# the last two characters of $a should have changed (into three)
is($a, "#!.\0\0erl");

$outfile = tempfile();

open(O, ">$outfile") || die "sysio.t: cannot write $outfile: $!";
binmode O;

select(O); $|=1; select(STDOUT);

# cannot write negative lengths
eval { syswrite(O, $x, -1) };
like($@, qr/^Negative length /);

# $x still intact
is($x, 'abc');

# $outfile still intact
ok(!-s $outfile);

# should not be able to write from after the buffer
eval { syswrite(O, $x, 1, 4) };
like($@, qr/^Offset outside string /);

# $x still intact
is($x, 'abc');

# but it should be ok to write from the end of the buffer
syswrite(O, $x, 0, 3);
syswrite(O, $x, 1, 3);

# $outfile still intact
if ($reopen) {  # must close file to update EOF marker for stat
  close O; open(O, ">>$outfile") || die "sysio.t: cannot write $outfile: $!";
  binmode O;
}
ok(!-s $outfile);

# should not be able to write from before the buffer

eval { syswrite(O, $x, 1, -4) };
like($@, qr/^Offset outside string /);

# $x still intact
is($x, 'abc');

# $outfile still intact
if ($reopen) {  # must close file to update EOF marker for stat
  close O; open(O, ">>$outfile") || die "sysio.t: cannot write $outfile: $!";
  binmode O;
}
ok(!-s $outfile);

# [perl #67912] syswrite prints garbage if called with empty scalar and non-zero offset
eval { my $buf = ''; syswrite(O, $buf, 1, 1) };
like($@, qr/^Offset outside string /);

# $x still intact
is($x, 'abc');

# $outfile still intact
if ($reopen) {  # must close file to update EOF marker for stat
  close O; open(O, ">>$outfile") || die "sysio.t: cannot write $outfile: $!";
  binmode O;
}
ok(!-s $outfile);

eval { my $buf = 'x'; syswrite(O, $buf, 1, 2) };
like($@, qr/^Offset outside string /);

# $x still intact
is($x, 'abc');

# $outfile still intact
if ($reopen) {  # must close file to update EOF marker for stat
  close O; open(O, ">>$outfile") || die "sysio.t: cannot write $outfile: $!";
  binmode O;
}
ok(!-s $outfile);

# default offset 0
if (syswrite(O, $a, 2) == 2){
  pass();
} else {
  diag($!);
  fail();
  # most other tests make no sense after e.g. "No space left on device"
  die $!;
}


# $a still intact
is($a, "#!.\0\0erl");

# $outfile should have grown now
if ($reopen) {  # must close file to update EOF marker for stat
  close O; open(O, ">>$outfile") || die "sysio.t: cannot write $outfile: $!";
  binmode O;
}
is(-s $outfile, 2);

# with offset
is(syswrite(O, $a, 2, 5), 2);

# $a still intact
is($a, "#!.\0\0erl");

# $outfile should have grown now
if ($reopen) {  # must close file to update EOF marker for stat
  close O; open(O, ">>$outfile") || die "sysio.t: cannot write $outfile: $!";
  binmode O;
}
is(-s $outfile, 4);

# with negative offset and a bit too much length
is(syswrite(O, $a, 5, -3), 3);

# $a still intact
is($a, "#!.\0\0erl");

# $outfile should have grown now
if ($reopen) {  # must close file to update EOF marker for stat
  close O; open(O, ">>$outfile") || die "sysio.t: cannot write $outfile: $!";
  binmode O;
}
is(-s $outfile, 7);

# with implicit length argument
is(syswrite(O, $x), 3);

# $a still intact
is($x, "abc");

# $outfile should have grown now
if ($reopen) {  # must close file to update EOF marker for stat
  close O; open(O, ">>$outfile") || die "sysio.t: cannot write $outfile: $!";
  binmode O;
}
is(-s $outfile, 10);

close(O);

open(I, $outfile) || die "sysio.t: cannot read $outfile: $!";
binmode I;

$b = 'xyz';

# reading too much only return as much as available
is(sysread(I, $b, 100), 10);

# this we should have
is($b, '#!ererlabc');

# test sysseek

is(sysseek(I, 2, 0), 2);
sysread(I, $b, 3);
is($b, 'ere');

is(sysseek(I, -2, 1), 3);
sysread(I, $b, 4);
is($b, 'rerl');

ok(sysseek(I, 0, 0) eq '0 but true');

ok(not defined sysseek(I, -1, 1));

close(I);

unlink_all $outfile;

chdir('..');

1;

# eof
