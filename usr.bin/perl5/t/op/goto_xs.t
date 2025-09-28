#!./perl
# tests for "goto &sub"-ing into XSUBs

# Note: This only tests things that should *work*.  At some point, it may
#       be worth while to write some failure tests for things that should
#       *break* (such as calls with wrong number of args).  For now, I'm
#       guessing that if all of these work correctly, the bad ones will
#       break correctly as well.

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
# turn warnings into fatal errors
    $SIG{__WARN__} = sub { die "WARNING: @_" } ;
    set_up_inc('../lib');
    skip_all_if_miniperl("no dynamic loading on miniperl, no Fcntl");
    require Fcntl;
}
use strict;
use warnings;
my $VALID;

plan(tests => 11);

# We don't know what symbols are defined in platform X's system headers.
# We don't even want to guess, because some platform out there will
# likely do the unthinkable.  However, Fcntl::S_IMODE(0)
# should always return a value.
# If this ceases to be the case, we're in trouble. =)
$VALID = 0;

### First, we check whether Fcntl::S_IMODE returns sane answers.
# Fcntl::S_IMODE(0) should always succeed.

my $value = Fcntl::S_IMODE($VALID);
isnt($value, undef, 'Sanity check broke, remaining tests will fail');

### OK, we're ready to do real tests.

sub goto_const { goto &Fcntl::S_IMODE; }

my $ret = goto_const($VALID);
is($ret, $value, 'goto &function_constant');

my $FNAME1 = 'Fcntl::S_IMODE';
sub goto_name1 { goto &$FNAME1; }

$ret = goto_name1($VALID);
is($ret, $value, 'goto &$function_package_and_name');

$ret = goto_name1($VALID);
is($ret, $value, 'goto &$function_package_and_name; again, with dirtier stack');
$ret = goto_name1($VALID);
is($ret, $value, 'goto &$function_package_and_name; again, with dirtier stack');

# test 
package Fcntl;
my $FNAME2 = 'S_IMODE';
sub goto_name2 { goto &$FNAME2; }
package main;

$ret = Fcntl::goto_name2($VALID);
is($ret, $value, 'goto &$function_name; from local package');

my $FREF = \&Fcntl::S_IMODE;
sub goto_ref { goto &$FREF; }

$ret = goto_ref($VALID);
is($ret, $value, 'goto &$function_ref');

### tests where the args are not on stack but in GvAV(defgv) (ie, @_)

sub call_goto_const { &goto_const; }

$ret = call_goto_const($VALID);
is($ret, $value, 'goto &function_constant; from a sub called without arglist');

# test "goto &$function_package_and_name" from a sub called without arglist
sub call_goto_name1 { &goto_name1; }

$ret = call_goto_name1($VALID);
is($ret, $value,
   'goto &$function_package_and_name; from a sub called without arglist');

sub call_goto_ref { &goto_ref; }

$ret = call_goto_ref($VALID);
is($ret, $value, 'goto &$function_ref; from a sub called without arglist');


BEGIN {
    use Config;
    if ($Config{extensions} =~ m{XS/APItest}) {
	eval q[use XS::APItest qw(mycroak); 1]
	    or die "use XS::APItest: $@\n";
    }
    else {
	*mycroak = sub { die @_ };
    }
}

sub goto_croak { goto &mycroak }

{
    my $e;
    for (1..4) {
	eval { goto_croak("boo$_\n") };
	$e .= $@;
    }
    is($e, "boo1\nboo2\nboo3\nboo4\n",
       '[perl #35878] croak in XS after goto segfaulted')
}

