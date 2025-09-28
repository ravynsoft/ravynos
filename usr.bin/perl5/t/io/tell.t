#!./perl

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
}

plan(36);

$TST = 'TST';

$Is_Dosish = ($^O eq 'MSWin32' or
              $^O eq 'os2' or $^O eq 'cygwin');

open($TST, 'harness') || (die "Can't open harness");
binmode $TST if $Is_Dosish;
ok(!eof(TST), "eof is false after open() non-empty file");

$firstline = <$TST>;
$secondpos = tell;

$x = 0;
while (<TST>) {
    if (eof) {$x++;}
}
is($x, 1, "only one eof is in the file");

$lastpos = tell;

ok(eof, "tell() doesn't change current state of eof");

ok(seek($TST,0,0), "set current position at beginning of the file");

ok(!eof, "reset at beginning of file clears eof flag");

is($firstline, <TST>, "first line is the same after open() and after seek()");

is($secondpos, tell, "position is the same after reading the first line");

ok(seek(TST,0,1), "move current position on +0");

ok(!eof($TST), "it doesn't set eof flag");

is($secondpos, tell, "it doesn't change tell position");

ok(seek(TST,0,2), "move current position at the end of the file");

is($lastpos, tell, "the position is the same as after reading whole file line by line");

ok(eof, "it sets eof flag");

ok($., "current line number \$. is not null");

$curline = $.;
open(OTHER, 'harness') || (die "Can't open harness: $!");
binmode OTHER if ($^O eq 'MSWin32');

{
    local($.);

    ok($., "open() doesn't change filehandler for \$.");

    tell OTHER;
    ok(!$., "tell() does change filehandler for \$.");

    $. = 5;
    scalar <OTHER>;
    is ($., 6, "reading of one line adds +1 to current line number \$.");
}

is($., $curline, "the 'local' correctly restores old value of filehandler for \$. when goes out of scope");

{
    local($.);

    scalar <OTHER>;
    is($., 7, "reading of one line inside 'local' change filehandler for \$.");
}

is($., $curline, "the 'local' correctly restores old value of filehandler for \$. when goes out of scope");

{
    local($.);

    tell OTHER;
    is($., 7, "tell() inside 'local' change filehandler for \$.");
}

close(OTHER);
{
    no warnings 'closed';
    is(tell(OTHER), -1, "tell() for closed file returns -1");
}
{
    no warnings 'unopened';
    # this must be a handle that has never been opened
    is(tell(UNOPENED), -1, "tell() for unopened file returns -1");
}

# ftell(STDIN) (or any std streams) is undefined, it can return -1 or
# something else.  ftell() on pipes, fifos, and sockets is defined to
# return -1.

my $written = tempfile();

close($TST);
open($tst,">$written")  || die "Cannot open $written:$!";
binmode $tst if $Is_Dosish;

is(tell($tst), 0, "tell() for new file returns 0");

print $tst "fred\n";

is(tell($tst), 5, 'tell() after writing "fred\n" returns 5');

print $tst "more\n";

is(tell($tst), 10, 'tell() after writing "more\n" returns 10');

close($tst);

open($tst,"+>>$written")  || die "Cannot open $written:$!";
binmode $tst if $Is_Dosish;

if (0) 
{
 # :stdio does not pass these so ignore them for now 

is(tell($tst), 0, 'tell() for open mode "+>>" returns 0');

$line = <$tst>;

is($line, "fred\n", 'check first line in mode "+>>"');

is(tell($tst), 5, "check tell() afrer reading first line");

}

print $tst "xxxx\n";

ok( tell($tst) == 15 ||
    tell($tst) == 5,
    'check tell() after writing "xxxx\n"'); # unset PERLIO or PERLIO=stdio (e.g. HP-UX, Solaris)

close($tst);

open($tst,">$written")  || die "Cannot open $written:$!";
print $tst "foobar";
close $tst;
open($tst,">>$written")  || die "Cannot open $written:$!";

# This test makes a questionable assumption that the file pointer will
# be at eof after opening a file but before seeking, reading, or writing.
# The POSIX standard is vague on this point.
# Cygwin and VOS differ from other implementations.

if (tell ($tst) == 6) {
  pass("check tell() after writing in mode '>>'");
}
else {
  if (($^O eq "cygwin") && (&PerlIO::get_layers($tst) eq 'stdio')) {
    fail "# TODO: file pointer not at eof";
  }
  elsif ($^O eq "vos") {
    fail "# TODO: Hit bug posix-2056. file pointer not at eof";
  }
  else {
    fail "file pointer not at eof";
  }
}

close $tst;

open FH, "test.pl";
$fh = *FH; # coercible glob
is(tell($fh), 0, "tell on coercible glob");
is(tell, 0, "argless tell after tell \$coercible");
tell *$fh;
is(tell, 0, "argless tell after tell *\$coercible");
eof $fh;
is(tell, 0, "argless tell after eof \$coercible");
eof *$fh;
is(tell, 0, "argless tell after eof *\$coercible");
seek $fh,0,0;
is(tell, 0, "argless tell after seek \$coercible...");
seek *$fh,0,0;
is(tell, 0, "argless tell after seek *\$coercible...");

{
    # [perl #133721]
    fresh_perl_is(<<'EOI', 'ok', {}, 'eof with no ${^LAST_FH}');
print "ok" if eof;
EOI
}
