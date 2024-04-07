#!./perl
#                              -*- Mode: Perl -*-
# closure.t:
#   Original written by Ulrich Pfeifer on 2 Jan 1997.
#   Greatly extended by Tom Phoenix <rootbeer@teleport.com> on 28 Jan 1997.
#
#   Run with -debug for debugging output.

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
}

use Config;

my $i = 1;
sub foo { $i = shift if @_; $i }

# no closure
is(foo, 1);
foo(2);
is(foo, 2);

# closure: lexical outside sub
my $foo = sub {$i = shift if @_; $i };
my $bar = sub {$i = shift if @_; $i };
is(&$foo(), 2);
&$foo(3);
is(&$foo(), 3);
# did the lexical change?
is(foo, 3, 'lexical changed');
is($i, 3, 'lexical changed');
# did the second closure notice?
is(&$bar(), 3, 'second closure noticed');

# closure: lexical inside sub
sub bar {
  my $i = shift;
  sub { $i = shift if @_; $i }
}

$foo = bar(4);
$bar = bar(5);
is(&$foo(), 4);
&$foo(6);
is(&$foo(), 6);
is(&$bar(), 5);

# nested closures
sub bizz {
  my $i = 7;
  if (@_) {
    my $i = shift;
    sub {$i = shift if @_; $i };
  } else {
    my $i = $i;
    sub {$i = shift if @_; $i };
  }
}
$foo = bizz();
$bar = bizz();
is(&$foo(), 7);
&$foo(8);
is(&$foo(), 8);
is(&$bar(), 7);

$foo = bizz(9);
$bar = bizz(10);
is(&$foo(11)-1, &$bar());

my @foo;
for (qw(0 1 2 3 4)) {
  my $i = $_;
  $foo[$_] = sub {$i = shift if @_; $i };
}

is(&{$foo[0]}(), 0);
is(&{$foo[1]}(), 1);
is(&{$foo[2]}(), 2);
is(&{$foo[3]}(), 3);
is(&{$foo[4]}(), 4);

for (0 .. 4) {
  &{$foo[$_]}(4-$_);
}

is(&{$foo[0]}(), 4);
is(&{$foo[1]}(), 3);
is(&{$foo[2]}(), 2);
is(&{$foo[3]}(), 1);
is(&{$foo[4]}(), 0);

sub barf {
  my @foo;
  for (qw(0 1 2 3 4)) {
    my $i = $_;
    $foo[$_] = sub {$i = shift if @_; $i };
  }
  @foo;
}

@foo = barf();
is(&{$foo[0]}(), 0);
is(&{$foo[1]}(), 1);
is(&{$foo[2]}(), 2);
is(&{$foo[3]}(), 3);
is(&{$foo[4]}(), 4);

for (0 .. 4) {
  &{$foo[$_]}(4-$_);
}

is(&{$foo[0]}(), 4);
is(&{$foo[1]}(), 3);
is(&{$foo[2]}(), 2);
is(&{$foo[3]}(), 1);
is(&{$foo[4]}(), 0);

# test if closures get created in optimized for loops

my %foo;
for my $n ('A'..'E') {
    $foo{$n} = sub { $n eq $_[0] };
}

ok(&{$foo{A}}('A'));
ok(&{$foo{B}}('B'));
ok(&{$foo{C}}('C'));
ok(&{$foo{D}}('D'));
ok(&{$foo{E}}('E'));

for my $n (0..4) {
    $foo[$n] = sub { $n == $_[0] };
}

ok(&{$foo[0]}(0));
ok(&{$foo[1]}(1));
ok(&{$foo[2]}(2));
ok(&{$foo[3]}(3));
ok(&{$foo[4]}(4));

for my $n (0..4) {
    $foo[$n] = sub {
                     # no intervening reference to $n here
                     sub { $n == $_[0] }
		   };
}

ok($foo[0]->()->(0));
ok($foo[1]->()->(1));
ok($foo[2]->()->(2));
ok($foo[3]->()->(3));
ok($foo[4]->()->(4));

{
    my $w;
    $w = sub {
	my ($i) = @_;
	is($i, 10);
	sub { $w };
    };
    $w->(10);
}

# Additional tests by Tom Phoenix <rootbeer@teleport.com>.

{
    use strict;

    our $test;
    my($debugging, %expected, $inner_type, $where_declared, $within);
    my($nc_attempt, $call_outer, $call_inner, $undef_outer);
    my($code, $inner_sub_test, $expected, $line, $errors, $output);
    my(@inners, $sub_test, $pid);
    $debugging = 1 if defined($ARGV[0]) and $ARGV[0] eq '-debug';

    # The expected values for these tests
    %expected = (
	'global_scalar'	=> 1001,
	'global_array'	=> 2101,
	'global_hash'	=> 3004,
	'fs_scalar'	=> 4001,
	'fs_array'	=> 5101,
	'fs_hash'	=> 6004,
	'sub_scalar'	=> 7001,
	'sub_array'	=> 8101,
	'sub_hash'	=> 9004,
	'foreach'	=> 10011,
    );

    # Our innermost sub is either named or anonymous
    for $inner_type (qw!named anon!) {
      # And it may be declared at filescope, within a named
      # sub, or within an anon sub
      for $where_declared (qw!filescope in_named in_anon!) {
	# And that, in turn, may be within a foreach loop,
	# a naked block, or another named sub
	for $within (qw!foreach naked other_sub!) {

	  my $test = curr_test();
	  # Here are a number of variables which show what's
	  # going on, in a way.
	  $nc_attempt = 0+		# Named closure attempted
	      ( ($inner_type eq 'named') ||
	      ($within eq 'other_sub') ) ;
	  $call_inner = 0+		# Need to call &inner
	      ( ($inner_type eq 'anon') &&
	      ($within eq 'other_sub') ) ;
	  $call_outer = 0+		# Need to call &outer or &$outer
	      ( ($inner_type eq 'anon') &&
	      ($within ne 'other_sub') ) ;
	  $undef_outer = 0+		# $outer is created but unused
	      ( ($where_declared eq 'in_anon') &&
	      (not $call_outer) ) ;

	  $code = "# This is a test script built by t/op/closure.t\n\n";

	  print <<"DEBUG_INFO" if $debugging;
# inner_type:     $inner_type 
# where_declared: $where_declared 
# within:         $within
# nc_attempt:     $nc_attempt
# call_inner:     $call_inner
# call_outer:     $call_outer
# undef_outer:    $undef_outer
DEBUG_INFO

	  $code .= <<"END_MARK_ONE";

BEGIN { \$SIG{__WARN__} = sub { 
    my \$msg = \$_[0];
END_MARK_ONE

	  $code .=  <<"END_MARK_TWO" if $nc_attempt;
    return if index(\$msg, 'will not stay shared') != -1;
    return if index(\$msg, 'is not available') != -1;
END_MARK_TWO

	  $code .= <<"END_MARK_THREE";		# Backwhack a lot!
    print "not ok: got unexpected warning \$msg\\n";
} }

require './test.pl';
curr_test($test);

# some of the variables which the closure will access
\$global_scalar = 1000;
\@global_array = (2000, 2100, 2200, 2300);
%global_hash = 3000..3009;

my \$fs_scalar = 4000;
my \@fs_array = (5000, 5100, 5200, 5300);
my %fs_hash = 6000..6009;

END_MARK_THREE

	  if ($where_declared eq 'filescope') {
	    # Nothing here
	  } elsif ($where_declared eq 'in_named') {
	    $code .= <<'END';
sub outer {
  my $sub_scalar = 7000;
  my @sub_array = (8000, 8100, 8200, 8300);
  my %sub_hash = 9000..9009;
END
    # }
	  } elsif ($where_declared eq 'in_anon') {
	    $code .= <<'END';
$outer = sub {
  my $sub_scalar = 7000;
  my @sub_array = (8000, 8100, 8200, 8300);
  my %sub_hash = 9000..9009;
END
    # }
	  } else {
	    die "What was $where_declared?"
	  }

	  if ($within eq 'foreach') {
	    $code .= "
      my \$foreach = 12000;
      my \@list = (10000, 10010);
      foreach \$foreach (\@list) {
    " # }
	  } elsif ($within eq 'naked') {
	    $code .= "  { # naked block\n"	# }
	  } elsif ($within eq 'other_sub') {
	    $code .= "  sub inner_sub {\n"	# }
	  } else {
	    die "What was $within?"
	  }

	  $sub_test = $test;
	  @inners = ( qw!global_scalar global_array global_hash! ,
	    qw!fs_scalar fs_array fs_hash! );
	  push @inners, 'foreach' if $within eq 'foreach';
	  if ($where_declared ne 'filescope') {
	    push @inners, qw!sub_scalar sub_array sub_hash!;
	  }
	  for $inner_sub_test (@inners) {

	    if ($inner_type eq 'named') {
	      $code .= "    sub named_$sub_test "
	    } elsif ($inner_type eq 'anon') {
	      $code .= "    \$anon_$sub_test = sub "
	    } else {
	      die "What was $inner_type?"
	    }

	    # Now to write the body of the test sub
	    if ($inner_sub_test eq 'global_scalar') {
	      $code .= '{ ++$global_scalar }'
	    } elsif ($inner_sub_test eq 'fs_scalar') {
	      $code .= '{ ++$fs_scalar }'
	    } elsif ($inner_sub_test eq 'sub_scalar') {
	      $code .= '{ ++$sub_scalar }'
	    } elsif ($inner_sub_test eq 'global_array') {
	      $code .= '{ ++$global_array[1] }'
	    } elsif ($inner_sub_test eq 'fs_array') {
	      $code .= '{ ++$fs_array[1] }'
	    } elsif ($inner_sub_test eq 'sub_array') {
	      $code .= '{ ++$sub_array[1] }'
	    } elsif ($inner_sub_test eq 'global_hash') {
	      $code .= '{ ++$global_hash{3002} }'
	    } elsif ($inner_sub_test eq 'fs_hash') {
	      $code .= '{ ++$fs_hash{6002} }'
	    } elsif ($inner_sub_test eq 'sub_hash') {
	      $code .= '{ ++$sub_hash{9002} }'
	    } elsif ($inner_sub_test eq 'foreach') {
	      $code .= '{ ++$foreach }'
	    } else {
	      die "What was $inner_sub_test?"
	    }
	  
	    # Close up
	    if ($inner_type eq 'anon') {
	      $code .= ';'
	    }
	    $code .= "\n";
	    $sub_test++;	# sub name sequence number

	  } # End of foreach $inner_sub_test

	  # Close up $within block		# {
	  $code .= "  }\n\n";

	  # Close up $where_declared block
	  if ($where_declared eq 'in_named') {	# {
	    $code .= "}\n\n";
	  } elsif ($where_declared eq 'in_anon') {	# {
	    $code .= "};\n\n";
	  }

	  # We may need to do something with the sub we just made...
	  $code .= "undef \$outer;\n" if $undef_outer;
	  $code .= "&inner_sub;\n" if $call_inner;
	  if ($call_outer) {
	    if ($where_declared eq 'in_named') {
	      $code .= "&outer;\n\n";
	    } elsif ($where_declared eq 'in_anon') {
	      $code .= "&\$outer;\n\n"
	    }
	  }

	  # Now, we can actually prep to run the tests.
	  for $inner_sub_test (@inners) {
	    $expected = $expected{$inner_sub_test} or
	      die "expected $inner_sub_test missing";

	    # Named closures won't access the expected vars
	    if ( $nc_attempt and 
		substr($inner_sub_test, 0, 4) eq "sub_" ) {
	      $expected = 1;
	    }

	    # If you make a sub within a foreach loop,
	    # what happens if it tries to access the 
	    # foreach index variable? If it's a named
	    # sub, it gets the var from "outside" the loop,
	    # but if it's anon, it gets the value to which
	    # the index variable is aliased.
	    #
	    # Of course, if the value was set only
	    # within another sub which was never called,
	    # the value has not been set yet.
	    #
	    if ($inner_sub_test eq 'foreach') {
	      if ($inner_type eq 'named') {
		if ($call_outer || ($where_declared eq 'filescope')) {
		  $expected = 12001
		} else {
		  $expected = 1
		}
	      }
	    }

	    # Here's the test:
	    my $desc = "$inner_type $where_declared $within $inner_sub_test";
	    if ($inner_type eq 'anon') {
	      $code .= "is(&\$anon_$test, $expected, '$desc');\n"
	    } else {
	      $code .= "is(&named_$test, $expected, '$desc');\n"
	    }
	    $test++;
	  }

	  if ($Config{d_fork} and $^O ne 'VMS' and $^O ne 'MSWin32') {
	    # Fork off a new perl to run the tests.
	    # (This is so we can catch spurious warnings.)
	    $| = 1; print ""; $| = 0; # flush output before forking
	    pipe READ, WRITE or die "Can't make pipe: $!";
	    pipe READ2, WRITE2 or die "Can't make second pipe: $!";
	    die "Can't fork: $!" unless defined($pid = open PERL, "|-");
	    unless ($pid) {
	      # Child process here. We're going to send errors back
	      # through the extra pipe.
	      close READ;
	      close READ2;
	      open STDOUT, ">&WRITE"  or die "Can't redirect STDOUT: $!";
	      open STDERR, ">&WRITE2" or die "Can't redirect STDERR: $!";
	      exec which_perl(), '-w', '-'
		or die "Can't exec perl: $!";
	    } else {
	      # Parent process here.
	      close WRITE;
	      close WRITE2;
	      print PERL $code;
	      close PERL;
	      { local $/;
	        $output = join '', <READ>;
	        $errors = join '', <READ2>; }
	      close READ;
	      close READ2;
	    }
	  } else {
	    # No fork().  Do it the hard way.
	    my $cmdfile = tempfile();
	    my $errfile = tempfile();
	    open CMD, ">$cmdfile"; print CMD $code; close CMD;
	    my $cmd = which_perl();
	    $cmd .= " -w $cmdfile 2>$errfile";
	    if ($^O eq 'VMS' or $^O eq 'MSWin32') {
	      # Use pipe instead of system so we don't inherit STD* from
	      # this process, and then foul our pipe back to parent by
	      # redirecting output in the child.
	      open PERL,"$cmd |" or die "Can't open pipe: $!\n";
	      { local $/; $output = join '', <PERL> }
	      close PERL;
	    } else {
	      my $outfile = tempfile();
	      system "$cmd >$outfile";
	      { local $/; open IN, $outfile; $output = <IN>; close IN }
	    }
	    if ($?) {
	      printf "not ok: exited with error code %04X\n", $?;
	      exit;
	    }
	    { local $/; open IN, $errfile; $errors = <IN>; close IN }
	  }
	  print $output;
	  curr_test($test);
	  print STDERR $errors;
	  # This has the side effect of alerting *our* test.pl to the state of
	  # what has just been passed to STDOUT, so that if anything there has
	  # failed, our test.pl will print a diagnostic and exit uncleanly.
	  unlike($output, qr/not ok/, 'All good');
	  is($errors, '', 'STDERR is silent');
	  if ($debugging && ($errors || $? || ($output =~ /not ok/))) {
	    my $lnum = 0;
	    for $line (split '\n', $code) {
	      printf "%3d:  %s\n", ++$lnum, $line;
	    }
	  }
	  is($?, 0, 'exited cleanly') or diag(sprintf "Error code $? = 0x%X", $?);
	  print '#', "-" x 30, "\n" if $debugging;

	}	# End of foreach $within
      }	# End of foreach $where_declared
    }	# End of foreach $inner_type

}

# The following dumps core with perl <= 5.8.0 (bugid 9535) ...
BEGIN { $vanishing_pad = sub { eval $_[0] } }
$some_var = 123;
is($vanishing_pad->('$some_var'), 123, 'RT #9535');

# ... and here's another coredump variant - this time we explicitly
# delete the sub rather than using a BEGIN ...

sub deleteme { $a = sub { eval '$newvar' } }
deleteme();
*deleteme = sub {}; # delete the sub
$newvar = 123; # realloc the SV of the freed CV
is($a->(), 123, 'RT #9535');

# ... and a further coredump variant - the fixup of the anon sub's
# CvOUTSIDE pointer when the middle eval is freed, wasn't good enough to
# survive the outer eval also being freed.

$x = 123;
$a = eval q(
    eval q[
	sub { eval '$x' }
    ]
);
@a = ('\1\1\1\1\1\1\1') x 100; # realloc recently-freed CVs
is($a->(), 123, 'RT #9535');

# this coredumped on <= 5.8.0 because evaling the closure caused
# an SvFAKE to be added to the outer anon's pad, which was then grown.
my $outer;
sub {
    my $x;
    $x = eval 'sub { $outer }';
    $x->();
    $a = [ 99 ];
    $x->();
}->();
pass();

# [perl #17605] found that an empty block called in scalar context
# can lead to stack corruption
{
    my $x = "foooobar";
    $x =~ s/o//eg;
    is($x, 'fbar', 'RT #17605');
}

# DAPM 24-Nov-02
# SvFAKE lexicals should be visible thoughout a function.
# On <= 5.8.0, the third test failed,  eg bugid #18286

{
    my $x = 1;
    sub fake {
		is(sub {eval'$x'}->(), 1, 'RT #18286');
	{ $x;	is(sub {eval'$x'}->(), 1, 'RT #18286'); }
		is(sub {eval'$x'}->(), 1, 'RT #18286');
    }
}
fake();

{
    $x = 1;
    my $x = 2;
    sub tmp { sub { eval '$x' } }
    my $a = tmp();
    undef &tmp;
    is($a->(), 2,
       "undefining a sub shouldn't alter visibility of outer lexicals");
}

# handy class: $x = Watch->new(\$foo,'bar')
# causes 'bar' to be appended to $foo when $x is destroyed
sub Watch::new { bless [ $_[1], $_[2] ], $_[0] }
sub Watch::DESTROY { ${$_[0][0]} .= $_[0][1] }

# bugid 1028:
# nested anon subs (and associated lexicals) not freed early enough

sub linger {
    my $x = Watch->new($_[0], '2');
    sub {
	$x;
	my $y;
	sub { $y; };
    };
}
{
    my $watch = '1';
    linger(\$watch);
    is($watch, '12', 'RT #1028');
}

# bugid 10085
# obj not freed early enough

sub linger2 { 
    my $obj = Watch->new($_[0], '2');
    sub { sub { $obj } };
}   
{
    my $watch = '1';
    linger2(\$watch);
    is($watch, 12, 'RT #10085');
}

# bugid 16302 - named subs didn't capture lexicals on behalf of inner subs

{
    my $x = 1;
    sub f16302 {
	sub {
	    is($x, 1, 'RT #16302');
	}->();
    }
}
f16302();

# The presence of an eval should turn cloneless anon subs into clonable
# subs - otherwise the CvOUTSIDE of that sub may be wrong

{
    my %a;
    for my $x (7,11) {
	$a{$x} = sub { $x=$x; sub { eval '$x' } };
    }
    is($a{7}->()->() + $a{11}->()->(), 18);
}

{
   # bugid #23265 - this used to coredump during destruction of PL_main_cv
   # and its children

    fresh_perl_is(<< '__EOF__', "yxx\n", {stderr => 1}, 'RT #23265');
        print
            sub {$_[0]->(@_)} -> (
                sub {
                    $_[1]
                        ?  $_[0]->($_[0], $_[1] - 1) .  sub {"x"}->()
                        : "y"
                },   
                2
            )
            , "\n"
        ;
__EOF__
}

{
    # bugid #24914 = used to coredump restoring PL_comppad in the
    # savestack, due to the early freeing of the anon closure

    fresh_perl_is('sub d {die} my $f; $f = sub {my $x=1; $f = 0; d}; eval{$f->()}; print qq(ok\n)',
		  "ok\n", {stderr => 1}, 'RT #24914');
}


# After newsub is redefined outside the BEGIN, its CvOUTSIDE should point
# to main rather than BEGIN, and BEGIN should be freed.

{
    my $flag = 0;
    sub  X::DESTROY { $flag = 1 }
    {
	my $x;
	BEGIN {$x = \&newsub }
	sub newsub {};
	$x = bless {}, 'X';
    }
    is($flag, 1);
}

sub f {
    my $x;
    format ff =
@
$r = \$x
.
}

{
    fileno ff;
    write ff;
    my $r1 = $r;
    write ff;
    my $r2 = $r;
    isnt($r1, $r2,
	 "don't copy a stale lexical; create a fresh undef one instead");
}

# test PL_cv_has_eval.  Any anon sub that could conceivably contain an
# eval, should be marked as cloneable

{

    my @s;
    push @s, sub {  eval '1' } for 1,2;
    isnt($s[0], $s[1], "cloneable with eval");
    @s = ();
    push @s, sub { use re 'eval'; my $x; s/$x/1/; } for 1,2;
    isnt($s[0], $s[1], "cloneable with use re eval");
    @s = ();
    push @s, sub { s/1/1/ee; } for 1,2;
    isnt($s[0], $s[1], "cloneable with //ee");
}

# [perl #89544]
{
   sub trace::DESTROY {
       push @trace::trace, "destroyed";
   }

   my $outer2 = sub {
       my $a = bless \my $dummy, trace::;

       my $outer = sub {
	   my $b;
	   my $inner = sub {
	       undef $b;
	   };

	   $a;

	   $inner
       };

       $outer->()
   };

   my $inner = $outer2->();
   is "@trace::trace", "destroyed",
      'closures only close over named variables, not entire subs';
}

# [perl #113812] Closure prototypes with no CvOUTSIDE (crash caused by the
#                fix for #89544)
do "./op/closure_test.pl" or die $@||$!;
is $closure_test::s2->()(), '10 cubes',
  'cloning closure proto with no CvOUTSIDE';

# Also brought up in #113812: Even when being cloned, a closure prototype
# might have its CvOUTSIDE pointing to the wrong thing.
{
    package main::113812;
    $s1 = sub {
	my $x = 3;
	$s2 = sub {
	    $x;
	    $s3 = sub { $x };
	};
    };
    $s1->();
    undef &$s1; # frees $s2's prototype, causing the $s3 proto to have its
                # CvOUTSIDE point to $s1
    ::is $s2->()(), 3, 'cloning closure proto whose CvOUTSIDE has changed';
}

# This should never emit two different values:
#     print $x, "\n";
#     print sub { $x }->(), "\n";
# This test case started to do just that in commit 33894c1aa3e
# (5.10.1/5.12.0):
sub mosquito {
    my $x if @_;
    return if @_;

    $x = 17;
    is sub { $x }->(), $x, 'closing over stale var in 2nd sub call';
}
mosquito(1);
mosquito;
# And this case in commit adf8f095c588 (5.14):
sub anything {
    my $x;
    sub gnat {
	$x = 3;
	is sub { $x }->(), $x,
	    'closing over stale var before 1st sub call';
    }
}
gnat();

# [perl #114018] Similar to the above, but with string eval
sub staleval {
    my $x if @_;
    return if @_;

    $x = 3;
    is eval '$x', $x, 'eval closing over stale var in active sub';
    return # 
}
staleval 1;
staleval;

# [perl #114888]
# Test that closure creation localises PL_comppad_name properly.  Usually
# at compile time a BEGIN block will localise PL_comppad_name for use, so
# pp_anoncode can mess with it without any visible effects.
# But inside a source filter, it affects the directly enclosing compila-
# tion scope.
SKIP: {
    skip_if_miniperl("no XS on miniperl (for source filters)");
    fresh_perl_is <<'    [perl #114888]', "ok\n", {stderr=>1},
	use strict;
	BEGIN {
	    package Foo;
	    use Filter::Util::Call;
	    sub import { filter_add( sub {
		my $status = filter_read();
		sub { $status };
		$status;
	    })}
	    Foo->import
	}
	my $x = "ok\n";	# stores $x in the wrong padnamelist
	print $x;	# cannot find it - strict violation
    [perl #114888]
        'closures in source filters do not interfere with pad names';
}

sub {
    my $f;
    sub test_ref_to_unavailable {
	my $ref = \$f;
        $$ref = 7;
        is $f, 7, 'taking a ref to unavailable var should not copy it';
    }
};
test_ref_to_unavailable();

done_testing();
