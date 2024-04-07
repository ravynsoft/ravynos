BEGIN {
    chdir 't' if -d 't/lib';
    @INC = '../lib' if -d 'lib';
    require Config; import Config;
    if (-d 'lib' and $Config{'extensions'} !~ /\bOS2(::|\/)REXX\b/) {
	print "1..0\n";
	exit 0;
    }
}

use OS2::REXX qw(:DEFAULT register);

$| = 1;				# Otherwise data from REXX may come first

print "1..18\n";

$n = 1;
sub do_me {
  print "ok $n\n";
  "OK";
}

@res = REXX_call(\&do_me);
print "ok 2\n";
@res == 1 ? print "ok 3\n" : print "not ok 3\n";
$res[0] eq "OK" ? print "ok 4\n" : print "not ok 4\n# `$res[0]'\n";

# Try again
$n = 5;
@res = REXX_call(\&do_me);
print "ok 6\n";
@res == 1 ? print "ok 7\n" : print "not ok 7\n";
$res[0] eq "OK" ? print "ok 8\n" : print "not ok 8\n# `$res[0]'\n";

REXX_call { print "ok 9\n" };
REXX_eval 'say "ok 10"';
# Try again
REXX_eval 'say "ok 11"';
print "ok 12\n" if REXX_eval("return 2 + 3") eq 5;
REXX_eval_with 'say myfunc()', myfunc => sub {"ok 13"};
REXX_eval_with "call myout 'ok'  14", myout => sub {print shift, "\n"};
REXX_eval_with "say 'ok 'myfunc(3,5)", myfunc => sub {shift() * shift()};

sub MYFUNC1 {shift}
sub MYFUNC2 {3 * shift}
REXX_eval_with "call myfunc
		say 'ok 'myfunc1(1)myfunc2(2)",
  myfunc => sub { register qw(myfunc1 myfunc2) };

REXX_eval_with "say 'ok 'myfunc(10,7)",
  myfunc => sub { REXX_eval "return $_[0] + $_[1]" };

sub MyFunc3 {print 'ok ', shift() + shift(), "\n"}
REXX_eval "address perleval\n'MyFunc3(10,8)'";
