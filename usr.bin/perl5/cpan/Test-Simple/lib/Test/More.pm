package Test::More;

use 5.006;
use strict;
use warnings;

#---- perlcritic exemptions. ----#

# We use a lot of subroutine prototypes
## no critic (Subroutines::ProhibitSubroutinePrototypes)

# Can't use Carp because it might cause C<use_ok()> to accidentally succeed
# even though the module being used forgot to use Carp.  Yes, this
# actually happened.
sub _carp {
    my( $file, $line ) = ( caller(1) )[ 1, 2 ];
    return warn @_, " at $file line $line\n";
}

our $VERSION = '1.302194';

use Test::Builder::Module;
our @ISA    = qw(Test::Builder::Module);
our @EXPORT = qw(ok use_ok require_ok
  is isnt like unlike is_deeply
  cmp_ok
  skip todo todo_skip
  pass fail
  eq_array eq_hash eq_set
  $TODO
  plan
  done_testing
  can_ok isa_ok new_ok
  diag note explain
  subtest
  BAIL_OUT
);

=head1 NAME

Test::More - yet another framework for writing test scripts

=head1 SYNOPSIS

  use Test::More tests => 23;
  # or
  use Test::More skip_all => $reason;
  # or
  use Test::More;   # see done_testing()

  require_ok( 'Some::Module' );

  # Various ways to say "ok"
  ok($got eq $expected, $test_name);

  is  ($got, $expected, $test_name);
  isnt($got, $expected, $test_name);

  # Rather than print STDERR "# here's what went wrong\n"
  diag("here's what went wrong");

  like  ($got, qr/expected/, $test_name);
  unlike($got, qr/expected/, $test_name);

  cmp_ok($got, '==', $expected, $test_name);

  is_deeply($got_complex_structure, $expected_complex_structure, $test_name);

  SKIP: {
      skip $why, $how_many unless $have_some_feature;

      ok( foo(),       $test_name );
      is( foo(42), 23, $test_name );
  };

  TODO: {
      local $TODO = $why;

      ok( foo(),       $test_name );
      is( foo(42), 23, $test_name );
  };

  can_ok($module, @methods);
  isa_ok($object, $class);

  pass($test_name);
  fail($test_name);

  BAIL_OUT($why);

  # UNIMPLEMENTED!!!
  my @status = Test::More::status;


=head1 DESCRIPTION

B<STOP!> If you're just getting started writing tests, have a look at
L<Test2::Suite> first.

This is a drop in replacement for Test::Simple which you can switch to once you
get the hang of basic testing.

The purpose of this module is to provide a wide range of testing
utilities.  Various ways to say "ok" with better diagnostics,
facilities to skip tests, test future features and compare complicated
data structures.  While you can do almost anything with a simple
C<ok()> function, it doesn't provide good diagnostic output.


=head2 I love it when a plan comes together

Before anything else, you need a testing plan.  This basically declares
how many tests your script is going to run to protect against premature
failure.

The preferred way to do this is to declare a plan when you C<use Test::More>.

  use Test::More tests => 23;

There are cases when you will not know beforehand how many tests your
script is going to run.  In this case, you can declare your tests at
the end.

  use Test::More;

  ... run your tests ...

  done_testing( $number_of_tests_run );

B<NOTE> C<done_testing()> should never be called in an C<END { ... }> block.

Sometimes you really don't know how many tests were run, or it's too
difficult to calculate.  In which case you can leave off
$number_of_tests_run.

In some cases, you'll want to completely skip an entire testing script.

  use Test::More skip_all => $skip_reason;

Your script will declare a skip with the reason why you skipped and
exit immediately with a zero (success).  See L<Test::Harness> for
details.

If you want to control what functions Test::More will export, you
have to use the 'import' option.  For example, to import everything
but 'fail', you'd do:

  use Test::More tests => 23, import => ['!fail'];

Alternatively, you can use the C<plan()> function.  Useful for when you
have to calculate the number of tests.

  use Test::More;
  plan tests => keys %Stuff * 3;

or for deciding between running the tests at all:

  use Test::More;
  if( $^O eq 'MacOS' ) {
      plan skip_all => 'Test irrelevant on MacOS';
  }
  else {
      plan tests => 42;
  }

=cut

sub plan {
    my $tb = Test::More->builder;

    return $tb->plan(@_);
}

# This implements "use Test::More 'no_diag'" but the behavior is
# deprecated.
sub import_extra {
    my $class = shift;
    my $list  = shift;

    my @other = ();
    my $idx   = 0;
    my $import;
    while( $idx <= $#{$list} ) {
        my $item = $list->[$idx];

        if( defined $item and $item eq 'no_diag' ) {
            $class->builder->no_diag(1);
        }
        elsif( defined $item and $item eq 'import' ) {
            if ($import) {
                push @$import, @{$list->[ ++$idx ]};
            }
            else {
                $import = $list->[ ++$idx ];
                push @other, $item, $import;
            }
        }
        else {
            push @other, $item;
        }

        $idx++;
    }

    @$list = @other;

    if ($class eq __PACKAGE__ && (!$import || grep $_ eq '$TODO', @$import)) {
        my $to = $class->builder->exported_to;
        no strict 'refs';
        *{"$to\::TODO"} = \our $TODO;
        if ($import) {
            @$import = grep $_ ne '$TODO', @$import;
        }
        else {
            push @$list, import => [grep $_ ne '$TODO', @EXPORT];
        }
    }

    return;
}

=over 4

=item B<done_testing>

    done_testing();
    done_testing($number_of_tests);

If you don't know how many tests you're going to run, you can issue
the plan when you're done running tests.

$number_of_tests is the same as C<plan()>, it's the number of tests you
expected to run.  You can omit this, in which case the number of tests
you ran doesn't matter, just the fact that your tests ran to
conclusion.

This is safer than and replaces the "no_plan" plan.

B<Note:> You must never put C<done_testing()> inside an C<END { ... }> block.
The plan is there to ensure your test does not exit before testing has
completed. If you use an END block you completely bypass this protection.

=back

=cut

sub done_testing {
    my $tb = Test::More->builder;
    $tb->done_testing(@_);
}

=head2 Test names

By convention, each test is assigned a number in order.  This is
largely done automatically for you.  However, it's often very useful to
assign a name to each test.  Which would you rather see:

  ok 4
  not ok 5
  ok 6

or

  ok 4 - basic multi-variable
  not ok 5 - simple exponential
  ok 6 - force == mass * acceleration

The later gives you some idea of what failed.  It also makes it easier
to find the test in your script, simply search for "simple
exponential".

All test functions take a name argument.  It's optional, but highly
suggested that you use it.

=head2 I'm ok, you're not ok.

The basic purpose of this module is to print out either "ok #" or "not
ok #" depending on if a given test succeeded or failed.  Everything
else is just gravy.

All of the following print "ok" or "not ok" depending on if the test
succeeded or failed.  They all also return true or false,
respectively.

=over 4

=item B<ok>

  ok($got eq $expected, $test_name);

This simply evaluates any expression (C<$got eq $expected> is just a
simple example) and uses that to determine if the test succeeded or
failed.  A true expression passes, a false one fails.  Very simple.

For example:

    ok( $exp{9} == 81,                   'simple exponential' );
    ok( Film->can('db_Main'),            'set_db()' );
    ok( $p->tests == 4,                  'saw tests' );
    ok( !grep(!defined $_, @items),      'all items defined' );

(Mnemonic:  "This is ok.")

$test_name is a very short description of the test that will be printed
out.  It makes it very easy to find a test in your script when it fails
and gives others an idea of your intentions.  $test_name is optional,
but we B<very> strongly encourage its use.

Should an C<ok()> fail, it will produce some diagnostics:

    not ok 18 - sufficient mucus
    #   Failed test 'sufficient mucus'
    #   in foo.t at line 42.

This is the same as L<Test::Simple>'s C<ok()> routine.

=cut

sub ok ($;$) {
    my( $test, $name ) = @_;
    my $tb = Test::More->builder;

    return $tb->ok( $test, $name );
}

=item B<is>

=item B<isnt>

  is  ( $got, $expected, $test_name );
  isnt( $got, $expected, $test_name );

Similar to C<ok()>, C<is()> and C<isnt()> compare their two arguments
with C<eq> and C<ne> respectively and use the result of that to
determine if the test succeeded or failed.  So these:

    # Is the ultimate answer 42?
    is( ultimate_answer(), 42,          "Meaning of Life" );

    # $foo isn't empty
    isnt( $foo, '',     "Got some foo" );

are similar to these:

    ok( ultimate_answer() eq 42,        "Meaning of Life" );
    ok( $foo ne '',     "Got some foo" );

C<undef> will only ever match C<undef>.  So you can test a value
against C<undef> like this:

    is($not_defined, undef, "undefined as expected");

(Mnemonic:  "This is that."  "This isn't that.")

So why use these?  They produce better diagnostics on failure.  C<ok()>
cannot know what you are testing for (beyond the name), but C<is()> and
C<isnt()> know what the test was and why it failed.  For example this
test:

    my $foo = 'waffle';  my $bar = 'yarblokos';
    is( $foo, $bar,   'Is foo the same as bar?' );

Will produce something like this:

    not ok 17 - Is foo the same as bar?
    #   Failed test 'Is foo the same as bar?'
    #   in foo.t at line 139.
    #          got: 'waffle'
    #     expected: 'yarblokos'

So you can figure out what went wrong without rerunning the test.

You are encouraged to use C<is()> and C<isnt()> over C<ok()> where possible,
however do not be tempted to use them to find out if something is
true or false!

  # XXX BAD!
  is( exists $brooklyn{tree}, 1, 'A tree grows in Brooklyn' );

This does not check if C<exists $brooklyn{tree}> is true, it checks if
it returns 1.  Very different.  Similar caveats exist for false and 0.
In these cases, use C<ok()>.

  ok( exists $brooklyn{tree},    'A tree grows in Brooklyn' );

A simple call to C<isnt()> usually does not provide a strong test but there
are cases when you cannot say much more about a value than that it is
different from some other value:

  new_ok $obj, "Foo";

  my $clone = $obj->clone;
  isa_ok $obj, "Foo", "Foo->clone";

  isnt $obj, $clone, "clone() produces a different object";

Historically we supported an C<isn't()> function as an alias of
C<isnt()>, however in Perl 5.37.9 support for the use of aprostrophe as
a package separator was deprecated and by Perl 5.42.0 support for it
will have been removed completely. Accordingly use of C<isn't()> is also
deprecated, and will produce warnings when used unless 'deprecated'
warnings are specifically disabled in the scope where it is used. You
are strongly advised to migrate to using C<isnt()> instead.

=cut

sub is ($$;$) {
    my $tb = Test::More->builder;

    return $tb->is_eq(@_);
}

sub isnt ($$;$) {
    my $tb = Test::More->builder;

    return $tb->isnt_eq(@_);
}

# Historically it was possible to use apostrophes as a package
# separator. make this available as isn't() for perl's that support it.
# However in 5.37.9 the apostrophe as a package separator was
# deprecated, so warn users of isn't() that they should use isnt()
# instead. We assume that if they are calling isn::t() they are doing so
# via isn't() as we have no way to be sure that they aren't spelling it
# with a double colon. We only trigger the warning if deprecation
# warnings are enabled, so the user can silence the warning if they
# wish.
sub isn::t {
    local ($@, $!, $?);
    if (warnings::enabled("deprecated")) {
        _carp
        "Use of apostrophe as package separator was deprecated in Perl 5.37.9,\n",
        "and will be removed in Perl 5.42.0.  You should change code that uses\n",
        "Test::More::isn't() to use Test::More::isnt() as a replacement";
    }
    goto &isnt;
}

=item B<like>

  like( $got, qr/expected/, $test_name );

Similar to C<ok()>, C<like()> matches $got against the regex C<qr/expected/>.

So this:

    like($got, qr/expected/, 'this is like that');

is similar to:

    ok( $got =~ m/expected/, 'this is like that');

(Mnemonic "This is like that".)

The second argument is a regular expression.  It may be given as a
regex reference (i.e. C<qr//>) or (for better compatibility with older
perls) as a string that looks like a regex (alternative delimiters are
currently not supported):

    like( $got, '/expected/', 'this is like that' );

Regex options may be placed on the end (C<'/expected/i'>).

Its advantages over C<ok()> are similar to that of C<is()> and C<isnt()>.  Better
diagnostics on failure.

=cut

sub like ($$;$) {
    my $tb = Test::More->builder;

    return $tb->like(@_);
}

=item B<unlike>

  unlike( $got, qr/expected/, $test_name );

Works exactly as C<like()>, only it checks if $got B<does not> match the
given pattern.

=cut

sub unlike ($$;$) {
    my $tb = Test::More->builder;

    return $tb->unlike(@_);
}

=item B<cmp_ok>

  cmp_ok( $got, $op, $expected, $test_name );

Halfway between C<ok()> and C<is()> lies C<cmp_ok()>.  This allows you
to compare two arguments using any binary perl operator.  The test
passes if the comparison is true and fails otherwise.

    # ok( $got eq $expected );
    cmp_ok( $got, 'eq', $expected, 'this eq that' );

    # ok( $got == $expected );
    cmp_ok( $got, '==', $expected, 'this == that' );

    # ok( $got && $expected );
    cmp_ok( $got, '&&', $expected, 'this && that' );
    ...etc...

Its advantage over C<ok()> is when the test fails you'll know what $got
and $expected were:

    not ok 1
    #   Failed test in foo.t at line 12.
    #     '23'
    #         &&
    #     undef

It's also useful in those cases where you are comparing numbers and
C<is()>'s use of C<eq> will interfere:

    cmp_ok( $big_hairy_number, '==', $another_big_hairy_number );

It's especially useful when comparing greater-than or smaller-than 
relation between values:

    cmp_ok( $some_value, '<=', $upper_limit );


=cut

sub cmp_ok($$$;$) {
    my $tb = Test::More->builder;

    return $tb->cmp_ok(@_);
}

=item B<can_ok>

  can_ok($module, @methods);
  can_ok($object, @methods);

Checks to make sure the $module or $object can do these @methods
(works with functions, too).

    can_ok('Foo', qw(this that whatever));

is almost exactly like saying:

    ok( Foo->can('this') && 
        Foo->can('that') && 
        Foo->can('whatever') 
      );

only without all the typing and with a better interface.  Handy for
quickly testing an interface.

No matter how many @methods you check, a single C<can_ok()> call counts
as one test.  If you desire otherwise, use:

    foreach my $meth (@methods) {
        can_ok('Foo', $meth);
    }

=cut

sub can_ok ($@) {
    my( $proto, @methods ) = @_;
    my $class = ref $proto || $proto;
    my $tb = Test::More->builder;

    unless($class) {
        my $ok = $tb->ok( 0, "->can(...)" );
        $tb->diag('    can_ok() called with empty class or reference');
        return $ok;
    }

    unless(@methods) {
        my $ok = $tb->ok( 0, "$class->can(...)" );
        $tb->diag('    can_ok() called with no methods');
        return $ok;
    }

    my @nok = ();
    foreach my $method (@methods) {
        $tb->_try( sub { $proto->can($method) } ) or push @nok, $method;
    }

    my $name = (@methods == 1) ? "$class->can('$methods[0]')" :
                                 "$class->can(...)"           ;

    my $ok = $tb->ok( !@nok, $name );

    $tb->diag( map "    $class->can('$_') failed\n", @nok );

    return $ok;
}

=item B<isa_ok>

  isa_ok($object,   $class, $object_name);
  isa_ok($subclass, $class, $object_name);
  isa_ok($ref,      $type,  $ref_name);

Checks to see if the given C<< $object->isa($class) >>.  Also checks to make
sure the object was defined in the first place.  Handy for this sort
of thing:

    my $obj = Some::Module->new;
    isa_ok( $obj, 'Some::Module' );

where you'd otherwise have to write

    my $obj = Some::Module->new;
    ok( defined $obj && $obj->isa('Some::Module') );

to safeguard against your test script blowing up.

You can also test a class, to make sure that it has the right ancestor:

    isa_ok( 'Vole', 'Rodent' );

It works on references, too:

    isa_ok( $array_ref, 'ARRAY' );

The diagnostics of this test normally just refer to 'the object'.  If
you'd like them to be more specific, you can supply an $object_name
(for example 'Test customer').

=cut

sub isa_ok ($$;$) {
    my( $thing, $class, $thing_name ) = @_;
    my $tb = Test::More->builder;

    my $whatami;
    if( !defined $thing ) {
        $whatami = 'undef';
    }
    elsif( ref $thing ) {
        $whatami = 'reference';

        local($@,$!);
        require Scalar::Util;
        if( Scalar::Util::blessed($thing) ) {
            $whatami = 'object';
        }
    }
    else {
        $whatami = 'class';
    }

    # We can't use UNIVERSAL::isa because we want to honor isa() overrides
    my( $rslt, $error ) = $tb->_try( sub { $thing->isa($class) } );

    if($error) {
        die <<WHOA unless $error =~ /^Can't (locate|call) method "isa"/;
WHOA! I tried to call ->isa on your $whatami and got some weird error.
Here's the error.
$error
WHOA
    }

    # Special case for isa_ok( [], "ARRAY" ) and like
    if( $whatami eq 'reference' ) {
        $rslt = UNIVERSAL::isa($thing, $class);
    }

    my($diag, $name);
    if( defined $thing_name ) {
        $name = "'$thing_name' isa '$class'";
        $diag = defined $thing ? "'$thing_name' isn't a '$class'" : "'$thing_name' isn't defined";
    }
    elsif( $whatami eq 'object' ) {
        my $my_class = ref $thing;
        $thing_name = qq[An object of class '$my_class'];
        $name = "$thing_name isa '$class'";
        $diag = "The object of class '$my_class' isn't a '$class'";
    }
    elsif( $whatami eq 'reference' ) {
        my $type = ref $thing;
        $thing_name = qq[A reference of type '$type'];
        $name = "$thing_name isa '$class'";
        $diag = "The reference of type '$type' isn't a '$class'";
    }
    elsif( $whatami eq 'undef' ) {
        $thing_name = 'undef';
        $name = "$thing_name isa '$class'";
        $diag = "$thing_name isn't defined";
    }
    elsif( $whatami eq 'class' ) {
        $thing_name = qq[The class (or class-like) '$thing'];
        $name = "$thing_name isa '$class'";
        $diag = "$thing_name isn't a '$class'";
    }
    else {
        die;
    }

    my $ok;
    if($rslt) {
        $ok = $tb->ok( 1, $name );
    }
    else {
        $ok = $tb->ok( 0, $name );
        $tb->diag("    $diag\n");
    }

    return $ok;
}

=item B<new_ok>

  my $obj = new_ok( $class );
  my $obj = new_ok( $class => \@args );
  my $obj = new_ok( $class => \@args, $object_name );

A convenience function which combines creating an object and calling
C<isa_ok()> on that object.

It is basically equivalent to:

    my $obj = $class->new(@args);
    isa_ok $obj, $class, $object_name;

If @args is not given, an empty list will be used.

This function only works on C<new()> and it assumes C<new()> will return
just a single object which isa C<$class>.

=cut

sub new_ok {
    my $tb = Test::More->builder;
    $tb->croak("new_ok() must be given at least a class") unless @_;

    my( $class, $args, $object_name ) = @_;

    $args ||= [];

    my $obj;
    my( $success, $error ) = $tb->_try( sub { $obj = $class->new(@$args); 1 } );
    if($success) {
        local $Test::Builder::Level = $Test::Builder::Level + 1;
        isa_ok $obj, $class, $object_name;
    }
    else {
        $class = 'undef' if !defined $class;
        $tb->ok( 0, "$class->new() died" );
        $tb->diag("    Error was:  $error");
    }

    return $obj;
}

=item B<subtest>

    subtest $name => \&code, @args;

C<subtest()> runs the &code as its own little test with its own plan and
its own result.  The main test counts this as a single test using the
result of the whole subtest to determine if its ok or not ok.

For example...

  use Test::More tests => 3;
 
  pass("First test");

  subtest 'An example subtest' => sub {
      plan tests => 2;

      pass("This is a subtest");
      pass("So is this");
  };

  pass("Third test");

This would produce.

  1..3
  ok 1 - First test
      # Subtest: An example subtest
      1..2
      ok 1 - This is a subtest
      ok 2 - So is this
  ok 2 - An example subtest
  ok 3 - Third test

A subtest may call C<skip_all>.  No tests will be run, but the subtest is
considered a skip.

  subtest 'skippy' => sub {
      plan skip_all => 'cuz I said so';
      pass('this test will never be run');
  };

Returns true if the subtest passed, false otherwise.

Due to how subtests work, you may omit a plan if you desire.  This adds an
implicit C<done_testing()> to the end of your subtest.  The following two
subtests are equivalent:

  subtest 'subtest with implicit done_testing()', sub {
      ok 1, 'subtests with an implicit done testing should work';
      ok 1, '... and support more than one test';
      ok 1, '... no matter how many tests are run';
  };

  subtest 'subtest with explicit done_testing()', sub {
      ok 1, 'subtests with an explicit done testing should work';
      ok 1, '... and support more than one test';
      ok 1, '... no matter how many tests are run';
      done_testing();
  };

Extra arguments given to C<subtest> are passed to the callback. For example:

    sub my_subtest {
        my $range = shift;
        ...
    }

    for my $range (1, 10, 100, 1000) {
        subtest "testing range $range", \&my_subtest, $range;
    }

=cut

sub subtest {
    my $tb = Test::More->builder;
    return $tb->subtest(@_);
}

=item B<pass>

=item B<fail>

  pass($test_name);
  fail($test_name);

Sometimes you just want to say that the tests have passed.  Usually
the case is you've got some complicated condition that is difficult to
wedge into an C<ok()>.  In this case, you can simply use C<pass()> (to
declare the test ok) or fail (for not ok).  They are synonyms for
C<ok(1)> and C<ok(0)>.

Use these very, very, very sparingly.

=cut

sub pass (;$) {
    my $tb = Test::More->builder;

    return $tb->ok( 1, @_ );
}

sub fail (;$) {
    my $tb = Test::More->builder;

    return $tb->ok( 0, @_ );
}

=back


=head2 Module tests

Sometimes you want to test if a module, or a list of modules, can
successfully load.  For example, you'll often want a first test which
simply loads all the modules in the distribution to make sure they
work before going on to do more complicated testing.

For such purposes we have C<use_ok> and C<require_ok>.

=over 4

=item B<require_ok>

   require_ok($module);
   require_ok($file);

Tries to C<require> the given $module or $file.  If it loads
successfully, the test will pass.  Otherwise it fails and displays the
load error.

C<require_ok> will guess whether the input is a module name or a
filename.

No exception will be thrown if the load fails.

    # require Some::Module
    require_ok "Some::Module";

    # require "Some/File.pl";
    require_ok "Some/File.pl";

    # stop testing if any of your modules will not load
    for my $module (@module) {
        require_ok $module or BAIL_OUT "Can't load $module";
    }

=cut

sub require_ok ($) {
    my($module) = shift;
    my $tb = Test::More->builder;

    my $pack = caller;

    # Try to determine if we've been given a module name or file.
    # Module names must be barewords, files not.
    $module = qq['$module'] unless _is_module_name($module);

    my $code = <<REQUIRE;
package $pack;
require $module;
1;
REQUIRE

    my( $eval_result, $eval_error ) = _eval($code);
    my $ok = $tb->ok( $eval_result, "require $module;" );

    unless($ok) {
        chomp $eval_error;
        $tb->diag(<<DIAGNOSTIC);
    Tried to require '$module'.
    Error:  $eval_error
DIAGNOSTIC

    }

    return $ok;
}

sub _is_module_name {
    my $module = shift;

    # Module names start with a letter.
    # End with an alphanumeric.
    # The rest is an alphanumeric or ::
    $module =~ s/\b::\b//g;

    return $module =~ /^[a-zA-Z]\w*$/ ? 1 : 0;
}


=item B<use_ok>

   BEGIN { use_ok($module); }
   BEGIN { use_ok($module, @imports); }

Like C<require_ok>, but it will C<use> the $module in question and
only loads modules, not files.

If you just want to test a module can be loaded, use C<require_ok>.

If you just want to load a module in a test, we recommend simply using
C<use> directly.  It will cause the test to stop.

It's recommended that you run C<use_ok()> inside a BEGIN block so its
functions are exported at compile-time and prototypes are properly
honored.

If @imports are given, they are passed through to the use.  So this:

   BEGIN { use_ok('Some::Module', qw(foo bar)) }

is like doing this:

   use Some::Module qw(foo bar);

Version numbers can be checked like so:

   # Just like "use Some::Module 1.02"
   BEGIN { use_ok('Some::Module', 1.02) }

Don't try to do this:

   BEGIN {
       use_ok('Some::Module');

       ...some code that depends on the use...
       ...happening at compile time...
   }

because the notion of "compile-time" is relative.  Instead, you want:

  BEGIN { use_ok('Some::Module') }
  BEGIN { ...some code that depends on the use... }

If you want the equivalent of C<use Foo ()>, use a module but not
import anything, use C<require_ok>.

  BEGIN { require_ok "Foo" }

=cut

sub use_ok ($;@) {
    my( $module, @imports ) = @_;
    @imports = () unless @imports;
    my $tb = Test::More->builder;

    my %caller;
    @caller{qw/pack file line sub args want eval req strict warn/} = caller(0);

    my ($pack, $filename, $line, $warn) = @caller{qw/pack file line warn/};
    $filename =~ y/\n\r/_/; # so it doesn't run off the "#line $line $f" line

    my $code;
    if( @imports == 1 and $imports[0] =~ /^\d+(?:\.\d+)?$/ ) {
        # probably a version check.  Perl needs to see the bare number
        # for it to work with non-Exporter based modules.
        $code = <<USE;
package $pack;
BEGIN { \${^WARNING_BITS} = \$args[-1] if defined \$args[-1] }
#line $line $filename
use $module $imports[0];
1;
USE
    }
    else {
        $code = <<USE;
package $pack;
BEGIN { \${^WARNING_BITS} = \$args[-1] if defined \$args[-1] }
#line $line $filename
use $module \@{\$args[0]};
1;
USE
    }

    my ($eval_result, $eval_error) = _eval($code, \@imports, $warn);
    my $ok = $tb->ok( $eval_result, "use $module;" );

    unless($ok) {
        chomp $eval_error;
        $@ =~ s{^BEGIN failed--compilation aborted at .*$}
                {BEGIN failed--compilation aborted at $filename line $line.}m;
        $tb->diag(<<DIAGNOSTIC);
    Tried to use '$module'.
    Error:  $eval_error
DIAGNOSTIC

    }

    return $ok;
}

sub _eval {
    my( $code, @args ) = @_;

    # Work around oddities surrounding resetting of $@ by immediately
    # storing it.
    my( $sigdie, $eval_result, $eval_error );
    {
        local( $@, $!, $SIG{__DIE__} );    # isolate eval
        $eval_result = eval $code;              ## no critic (BuiltinFunctions::ProhibitStringyEval)
        $eval_error  = $@;
        $sigdie      = $SIG{__DIE__} || undef;
    }
    # make sure that $code got a chance to set $SIG{__DIE__}
    $SIG{__DIE__} = $sigdie if defined $sigdie;

    return( $eval_result, $eval_error );
}


=back


=head2 Complex data structures

Not everything is a simple eq check or regex.  There are times you
need to see if two data structures are equivalent.  For these
instances Test::More provides a handful of useful functions.

B<NOTE> I'm not quite sure what will happen with filehandles.

=over 4

=item B<is_deeply>

  is_deeply( $got, $expected, $test_name );

Similar to C<is()>, except that if $got and $expected are references, it
does a deep comparison walking each data structure to see if they are
equivalent.  If the two structures are different, it will display the
place where they start differing.

C<is_deeply()> compares the dereferenced values of references, the
references themselves (except for their type) are ignored.  This means
aspects such as blessing and ties are not considered "different".

C<is_deeply()> currently has very limited handling of function reference
and globs.  It merely checks if they have the same referent.  This may
improve in the future.

L<Test::Differences> and L<Test::Deep> provide more in-depth functionality
along these lines.

B<NOTE> is_deeply() has limitations when it comes to comparing strings and
refs:

    my $path = path('.');
    my $hash = {};
    is_deeply( $path, "$path" ); # ok
    is_deeply( $hash, "$hash" ); # fail

This happens because is_deeply will unoverload all arguments unconditionally.
It is probably best not to use is_deeply with overloading. For legacy reasons
this is not likely to ever be fixed. If you would like a much better tool for
this you should see L<Test2::Suite> Specifically L<Test2::Tools::Compare> has
an C<is()> function that works like C<is_deeply> with many improvements.

=cut

our( @Data_Stack, %Refs_Seen );
my $DNE = bless [], 'Does::Not::Exist';

sub _dne {
    return ref $_[0] eq ref $DNE;
}

## no critic (Subroutines::RequireArgUnpacking)
sub is_deeply {
    my $tb = Test::More->builder;

    unless( @_ == 2 or @_ == 3 ) {
        my $msg = <<'WARNING';
is_deeply() takes two or three args, you gave %d.
This usually means you passed an array or hash instead 
of a reference to it
WARNING
        chop $msg;    # clip off newline so carp() will put in line/file

        _carp sprintf $msg, scalar @_;

        return $tb->ok(0);
    }

    my( $got, $expected, $name ) = @_;

    $tb->_unoverload_str( \$expected, \$got );

    my $ok;
    if( !ref $got and !ref $expected ) {    # neither is a reference
        $ok = $tb->is_eq( $got, $expected, $name );
    }
    elsif( !ref $got xor !ref $expected ) {    # one's a reference, one isn't
        $ok = $tb->ok( 0, $name );
        $tb->diag( _format_stack({ vals => [ $got, $expected ] }) );
    }
    else {                                     # both references
        local @Data_Stack = ();
        if( _deep_check( $got, $expected ) ) {
            $ok = $tb->ok( 1, $name );
        }
        else {
            $ok = $tb->ok( 0, $name );
            $tb->diag( _format_stack(@Data_Stack) );
        }
    }

    return $ok;
}

sub _format_stack {
    my(@Stack) = @_;

    my $var       = '$FOO';
    my $did_arrow = 0;
    foreach my $entry (@Stack) {
        my $type = $entry->{type} || '';
        my $idx = $entry->{'idx'};
        if( $type eq 'HASH' ) {
            $var .= "->" unless $did_arrow++;
            $var .= "{$idx}";
        }
        elsif( $type eq 'ARRAY' ) {
            $var .= "->" unless $did_arrow++;
            $var .= "[$idx]";
        }
        elsif( $type eq 'REF' ) {
            $var = "\${$var}";
        }
    }

    my @vals = @{ $Stack[-1]{vals} }[ 0, 1 ];
    my @vars = ();
    ( $vars[0] = $var ) =~ s/\$FOO/     \$got/;
    ( $vars[1] = $var ) =~ s/\$FOO/\$expected/;

    my $out = "Structures begin differing at:\n";
    foreach my $idx ( 0 .. $#vals ) {
        my $val = $vals[$idx];
        $vals[$idx]
          = !defined $val ? 'undef'
          : _dne($val)    ? "Does not exist"
          : ref $val      ? "$val"
          :                 "'$val'";
    }

    $out .= "$vars[0] = $vals[0]\n";
    $out .= "$vars[1] = $vals[1]\n";

    $out =~ s/^/    /msg;
    return $out;
}

sub _type {
    my $thing = shift;

    return '' if !ref $thing;

    for my $type (qw(Regexp ARRAY HASH REF SCALAR GLOB CODE VSTRING)) {
        return $type if UNIVERSAL::isa( $thing, $type );
    }

    return '';
}

=back


=head2 Diagnostics

If you pick the right test function, you'll usually get a good idea of
what went wrong when it failed.  But sometimes it doesn't work out
that way.  So here we have ways for you to write your own diagnostic
messages which are safer than just C<print STDERR>.

=over 4

=item B<diag>

  diag(@diagnostic_message);

Prints a diagnostic message which is guaranteed not to interfere with
test output.  Like C<print> @diagnostic_message is simply concatenated
together.

Returns false, so as to preserve failure.

Handy for this sort of thing:

    ok( grep(/foo/, @users), "There's a foo user" ) or
        diag("Since there's no foo, check that /etc/bar is set up right");

which would produce:

    not ok 42 - There's a foo user
    #   Failed test 'There's a foo user'
    #   in foo.t at line 52.
    # Since there's no foo, check that /etc/bar is set up right.

You might remember C<ok() or diag()> with the mnemonic C<open() or
die()>.

B<NOTE> The exact formatting of the diagnostic output is still
changing, but it is guaranteed that whatever you throw at it won't
interfere with the test.

=item B<note>

  note(@diagnostic_message);

Like C<diag()>, except the message will not be seen when the test is run
in a harness.  It will only be visible in the verbose TAP stream.

Handy for putting in notes which might be useful for debugging, but
don't indicate a problem.

    note("Tempfile is $tempfile");

=cut

sub diag {
    return Test::More->builder->diag(@_);
}

sub note {
    return Test::More->builder->note(@_);
}

=item B<explain>

  my @dump = explain @diagnostic_message;

Will dump the contents of any references in a human readable format.
Usually you want to pass this into C<note> or C<diag>.

Handy for things like...

    is_deeply($have, $want) || diag explain $have;

or

    note explain \%args;
    Some::Class->method(%args);

=cut

sub explain {
    return Test::More->builder->explain(@_);
}

=back


=head2 Conditional tests

Sometimes running a test under certain conditions will cause the
test script to die.  A certain function or method isn't implemented
(such as C<fork()> on MacOS), some resource isn't available (like a 
net connection) or a module isn't available.  In these cases it's
necessary to skip tests, or declare that they are supposed to fail
but will work in the future (a todo test).

For more details on the mechanics of skip and todo tests see
L<Test::Harness>.

The way Test::More handles this is with a named block.  Basically, a
block of tests which can be skipped over or made todo.  It's best if I
just show you...

=over 4

=item B<SKIP: BLOCK>

  SKIP: {
      skip $why, $how_many if $condition;

      ...normal testing code goes here...
  }

This declares a block of tests that might be skipped, $how_many tests
there are, $why and under what $condition to skip them.  An example is
the easiest way to illustrate:

    SKIP: {
        eval { require HTML::Lint };

        skip "HTML::Lint not installed", 2 if $@;

        my $lint = new HTML::Lint;
        isa_ok( $lint, "HTML::Lint" );

        $lint->parse( $html );
        is( $lint->errors, 0, "No errors found in HTML" );
    }

If the user does not have HTML::Lint installed, the whole block of
code I<won't be run at all>.  Test::More will output special ok's
which Test::Harness interprets as skipped, but passing, tests.

It's important that $how_many accurately reflects the number of tests
in the SKIP block so the # of tests run will match up with your plan.
If your plan is C<no_plan> $how_many is optional and will default to 1.

It's perfectly safe to nest SKIP blocks.  Each SKIP block must have
the label C<SKIP>, or Test::More can't work its magic.

You don't skip tests which are failing because there's a bug in your
program, or for which you don't yet have code written.  For that you
use TODO.  Read on.

=cut

## no critic (Subroutines::RequireFinalReturn)
sub skip {
    my( $why, $how_many ) = @_;
    my $tb = Test::More->builder;

    # If the plan is set, and is static, then skip needs a count. If the plan
    # is 'no_plan' we are fine. As well if plan is undefined then we are
    # waiting for done_testing.
    unless (defined $how_many) {
        my $plan = $tb->has_plan;
        _carp "skip() needs to know \$how_many tests are in the block"
            if $plan && $plan =~ m/^\d+$/;
        $how_many = 1;
    }

    if( defined $how_many and $how_many =~ /\D/ ) {
        _carp
          "skip() was passed a non-numeric number of tests.  Did you get the arguments backwards?";
        $how_many = 1;
    }

    for( 1 .. $how_many ) {
        $tb->skip($why);
    }

    no warnings 'exiting';
    last SKIP;
}

=item B<TODO: BLOCK>

    TODO: {
        local $TODO = $why if $condition;

        ...normal testing code goes here...
    }

Declares a block of tests you expect to fail and $why.  Perhaps it's
because you haven't fixed a bug or haven't finished a new feature:

    TODO: {
        local $TODO = "URI::Geller not finished";

        my $card = "Eight of clubs";
        is( URI::Geller->your_card, $card, 'Is THIS your card?' );

        my $spoon;
        URI::Geller->bend_spoon;
        is( $spoon, 'bent',    "Spoon bending, that's original" );
    }

With a todo block, the tests inside are expected to fail.  Test::More
will run the tests normally, but print out special flags indicating
they are "todo".  L<Test::Harness> will interpret failures as being ok.
Should anything succeed, it will report it as an unexpected success.
You then know the thing you had todo is done and can remove the
TODO flag.

The nice part about todo tests, as opposed to simply commenting out a
block of tests, is that it is like having a programmatic todo list.  You know
how much work is left to be done, you're aware of what bugs there are,
and you'll know immediately when they're fixed.

Once a todo test starts succeeding, simply move it outside the block.
When the block is empty, delete it.

Note that, if you leave $TODO unset or undef, Test::More reports failures
as normal. This can be useful to mark the tests as expected to fail only
in certain conditions, e.g.:

    TODO: {
        local $TODO = "$^O doesn't work yet. :(" if !_os_is_supported($^O);

        ...
    }

=item B<todo_skip>

    TODO: {
        todo_skip $why, $how_many if $condition;

        ...normal testing code...
    }

With todo tests, it's best to have the tests actually run.  That way
you'll know when they start passing.  Sometimes this isn't possible.
Often a failing test will cause the whole program to die or hang, even
inside an C<eval BLOCK> with and using C<alarm>.  In these extreme
cases you have no choice but to skip over the broken tests entirely.

The syntax and behavior is similar to a C<SKIP: BLOCK> except the
tests will be marked as failing but todo.  L<Test::Harness> will
interpret them as passing.

=cut

sub todo_skip {
    my( $why, $how_many ) = @_;
    my $tb = Test::More->builder;

    unless( defined $how_many ) {
        # $how_many can only be avoided when no_plan is in use.
        _carp "todo_skip() needs to know \$how_many tests are in the block"
          unless $tb->has_plan eq 'no_plan';
        $how_many = 1;
    }

    for( 1 .. $how_many ) {
        $tb->todo_skip($why);
    }

    no warnings 'exiting';
    last TODO;
}

=item When do I use SKIP vs. TODO?

B<If it's something the user might not be able to do>, use SKIP.
This includes optional modules that aren't installed, running under
an OS that doesn't have some feature (like C<fork()> or symlinks), or maybe
you need an Internet connection and one isn't available.

B<If it's something the programmer hasn't done yet>, use TODO.  This
is for any code you haven't written yet, or bugs you have yet to fix,
but want to put tests in your testing script (always a good idea).


=back


=head2 Test control

=over 4

=item B<BAIL_OUT>

    BAIL_OUT($reason);

Indicates to the harness that things are going so badly all testing
should terminate.  This includes the running of any additional test scripts.

This is typically used when testing cannot continue such as a critical
module failing to compile or a necessary external utility not being
available such as a database connection failing.

The test will exit with 255.

For even better control look at L<Test::Most>.

=cut

sub BAIL_OUT {
    my $reason = shift;
    my $tb     = Test::More->builder;

    $tb->BAIL_OUT($reason);
}

=back


=head2 Discouraged comparison functions

The use of the following functions is discouraged as they are not
actually testing functions and produce no diagnostics to help figure
out what went wrong.  They were written before C<is_deeply()> existed
because I couldn't figure out how to display a useful diff of two
arbitrary data structures.

These functions are usually used inside an C<ok()>.

    ok( eq_array(\@got, \@expected) );

C<is_deeply()> can do that better and with diagnostics.  

    is_deeply( \@got, \@expected );

They may be deprecated in future versions.

=over 4

=item B<eq_array>

  my $is_eq = eq_array(\@got, \@expected);

Checks if two arrays are equivalent.  This is a deep check, so
multi-level structures are handled correctly.

=cut

#'#
sub eq_array {
    local @Data_Stack = ();
    _deep_check(@_);
}

sub _eq_array {
    my( $a1, $a2 ) = @_;

    if( grep _type($_) ne 'ARRAY', $a1, $a2 ) {
        warn "eq_array passed a non-array ref";
        return 0;
    }

    return 1 if $a1 eq $a2;

    my $ok = 1;
    my $max = $#$a1 > $#$a2 ? $#$a1 : $#$a2;
    for( 0 .. $max ) {
        my $e1 = $_ > $#$a1 ? $DNE : $a1->[$_];
        my $e2 = $_ > $#$a2 ? $DNE : $a2->[$_];

        next if _equal_nonrefs($e1, $e2);

        push @Data_Stack, { type => 'ARRAY', idx => $_, vals => [ $e1, $e2 ] };
        $ok = _deep_check( $e1, $e2 );
        pop @Data_Stack if $ok;

        last unless $ok;
    }

    return $ok;
}

sub _equal_nonrefs {
    my( $e1, $e2 ) = @_;

    return if ref $e1 or ref $e2;

    if ( defined $e1 ) {
        return 1 if defined $e2 and $e1 eq $e2;
    }
    else {
        return 1 if !defined $e2;
    }

    return;
}

sub _deep_check {
    my( $e1, $e2 ) = @_;
    my $tb = Test::More->builder;

    my $ok = 0;

    # Effectively turn %Refs_Seen into a stack.  This avoids picking up
    # the same referenced used twice (such as [\$a, \$a]) to be considered
    # circular.
    local %Refs_Seen = %Refs_Seen;

    {
        $tb->_unoverload_str( \$e1, \$e2 );

        # Either they're both references or both not.
        my $same_ref = !( !ref $e1 xor !ref $e2 );
        my $not_ref = ( !ref $e1 and !ref $e2 );

        if( defined $e1 xor defined $e2 ) {
            $ok = 0;
        }
        elsif( !defined $e1 and !defined $e2 ) {
            # Shortcut if they're both undefined.
            $ok = 1;
        }
        elsif( _dne($e1) xor _dne($e2) ) {
            $ok = 0;
        }
        elsif( $same_ref and( $e1 eq $e2 ) ) {
            $ok = 1;
        }
        elsif($not_ref) {
            push @Data_Stack, { type => '', vals => [ $e1, $e2 ] };
            $ok = 0;
        }
        else {
            if( $Refs_Seen{$e1} ) {
                return $Refs_Seen{$e1} eq $e2;
            }
            else {
                $Refs_Seen{$e1} = "$e2";
            }

            my $type = _type($e1);
            $type = 'DIFFERENT' unless _type($e2) eq $type;

            if( $type eq 'DIFFERENT' ) {
                push @Data_Stack, { type => $type, vals => [ $e1, $e2 ] };
                $ok = 0;
            }
            elsif( $type eq 'ARRAY' ) {
                $ok = _eq_array( $e1, $e2 );
            }
            elsif( $type eq 'HASH' ) {
                $ok = _eq_hash( $e1, $e2 );
            }
            elsif( $type eq 'REF' ) {
                push @Data_Stack, { type => $type, vals => [ $e1, $e2 ] };
                $ok = _deep_check( $$e1, $$e2 );
                pop @Data_Stack if $ok;
            }
            elsif( $type eq 'SCALAR' ) {
                push @Data_Stack, { type => 'REF', vals => [ $e1, $e2 ] };
                $ok = _deep_check( $$e1, $$e2 );
                pop @Data_Stack if $ok;
            }
            elsif($type) {
                push @Data_Stack, { type => $type, vals => [ $e1, $e2 ] };
                $ok = 0;
            }
            else {
                _whoa( 1, "No type in _deep_check" );
            }
        }
    }

    return $ok;
}

sub _whoa {
    my( $check, $desc ) = @_;
    if($check) {
        die <<"WHOA";
WHOA!  $desc
This should never happen!  Please contact the author immediately!
WHOA
    }
}

=item B<eq_hash>

  my $is_eq = eq_hash(\%got, \%expected);

Determines if the two hashes contain the same keys and values.  This
is a deep check.

=cut

sub eq_hash {
    local @Data_Stack = ();
    return _deep_check(@_);
}

sub _eq_hash {
    my( $a1, $a2 ) = @_;

    if( grep _type($_) ne 'HASH', $a1, $a2 ) {
        warn "eq_hash passed a non-hash ref";
        return 0;
    }

    return 1 if $a1 eq $a2;

    my $ok = 1;
    my $bigger = keys %$a1 > keys %$a2 ? $a1 : $a2;
    foreach my $k ( keys %$bigger ) {
        my $e1 = exists $a1->{$k} ? $a1->{$k} : $DNE;
        my $e2 = exists $a2->{$k} ? $a2->{$k} : $DNE;

        next if _equal_nonrefs($e1, $e2);

        push @Data_Stack, { type => 'HASH', idx => $k, vals => [ $e1, $e2 ] };
        $ok = _deep_check( $e1, $e2 );
        pop @Data_Stack if $ok;

        last unless $ok;
    }

    return $ok;
}

=item B<eq_set>

  my $is_eq = eq_set(\@got, \@expected);

Similar to C<eq_array()>, except the order of the elements is B<not>
important.  This is a deep check, but the irrelevancy of order only
applies to the top level.

    ok( eq_set(\@got, \@expected) );

Is better written:

    is_deeply( [sort @got], [sort @expected] );

B<NOTE> By historical accident, this is not a true set comparison.
While the order of elements does not matter, duplicate elements do.

B<NOTE> C<eq_set()> does not know how to deal with references at the top
level.  The following is an example of a comparison which might not work:

    eq_set([\1, \2], [\2, \1]);

L<Test::Deep> contains much better set comparison functions.

=cut

sub eq_set {
    my( $a1, $a2 ) = @_;
    return 0 unless @$a1 == @$a2;

    no warnings 'uninitialized';

    # It really doesn't matter how we sort them, as long as both arrays are
    # sorted with the same algorithm.
    #
    # Ensure that references are not accidentally treated the same as a
    # string containing the reference.
    #
    # Have to inline the sort routine due to a threading/sort bug.
    # See [rt.cpan.org 6782]
    #
    # I don't know how references would be sorted so we just don't sort
    # them.  This means eq_set doesn't really work with refs.
    return eq_array(
        [ grep( ref, @$a1 ), sort( grep( !ref, @$a1 ) ) ],
        [ grep( ref, @$a2 ), sort( grep( !ref, @$a2 ) ) ],
    );
}

=back


=head2 Extending and Embedding Test::More

Sometimes the Test::More interface isn't quite enough.  Fortunately,
Test::More is built on top of L<Test::Builder> which provides a single,
unified backend for any test library to use.  This means two test
libraries which both use L<Test::Builder> B<can> be used together in the
same program.

If you simply want to do a little tweaking of how the tests behave,
you can access the underlying L<Test::Builder> object like so:

=over 4

=item B<builder>

    my $test_builder = Test::More->builder;

Returns the L<Test::Builder> object underlying Test::More for you to play
with.


=back


=head1 EXIT CODES

If all your tests passed, L<Test::Builder> will exit with zero (which is
normal).  If anything failed it will exit with how many failed.  If
you run less (or more) tests than you planned, the missing (or extras)
will be considered failures.  If no tests were ever run L<Test::Builder>
will throw a warning and exit with 255.  If the test died, even after
having successfully completed all its tests, it will still be
considered a failure and will exit with 255.

So the exit codes are...

    0                   all tests successful
    255                 test died or all passed but wrong # of tests run
    any other number    how many failed (including missing or extras)

If you fail more than 254 tests, it will be reported as 254.

B<NOTE>  This behavior may go away in future versions.


=head1 COMPATIBILITY

Test::More works with Perls as old as 5.8.1.

Thread support is not very reliable before 5.10.1, but that's
because threads are not very reliable before 5.10.1.

Although Test::More has been a core module in versions of Perl since 5.6.2, Test::More has evolved since then, and not all of the features you're used to will be present in the shipped version of Test::More. If you are writing a module, don't forget to indicate in your package metadata the minimum version of Test::More that you require. For instance, if you want to use C<done_testing()> but want your test script to run on Perl 5.10.0, you will need to explicitly require Test::More > 0.88.

Key feature milestones include:

=over 4

=item subtests

Subtests were released in Test::More 0.94, which came with Perl 5.12.0. Subtests did not implicitly call C<done_testing()> until 0.96; the first Perl with that fix was Perl 5.14.0 with 0.98.

=item C<done_testing()>

This was released in Test::More 0.88 and first shipped with Perl in 5.10.1 as part of Test::More 0.92. 

=item C<cmp_ok()>

Although C<cmp_ok()> was introduced in 0.40, 0.86 fixed an important bug to make it safe for overloaded objects; the fixed first shipped with Perl in 5.10.1 as part of Test::More 0.92.

=item C<new_ok()> C<note()> and C<explain()>

These were was released in Test::More 0.82, and first shipped with Perl in 5.10.1 as part of Test::More 0.92. 

=back

There is a full version history in the Changes file, and the Test::More versions included as core can be found using L<Module::CoreList>:

    $ corelist -a Test::More


=head1 CAVEATS and NOTES

=over 4

=item utf8 / "Wide character in print"

If you use utf8 or other non-ASCII characters with Test::More you
might get a "Wide character in print" warning.  Using
C<< binmode STDOUT, ":utf8" >> will not fix it.
L<Test::Builder> (which powers
Test::More) duplicates STDOUT and STDERR.  So any changes to them,
including changing their output disciplines, will not be seen by
Test::More.

One work around is to apply encodings to STDOUT and STDERR as early
as possible and before Test::More (or any other Test module) loads.

    use open ':std', ':encoding(utf8)';
    use Test::More;

A more direct work around is to change the filehandles used by
L<Test::Builder>.

    my $builder = Test::More->builder;
    binmode $builder->output,         ":encoding(utf8)";
    binmode $builder->failure_output, ":encoding(utf8)";
    binmode $builder->todo_output,    ":encoding(utf8)";


=item Overloaded objects

String overloaded objects are compared B<as strings> (or in C<cmp_ok()>'s
case, strings or numbers as appropriate to the comparison op).  This
prevents Test::More from piercing an object's interface allowing
better blackbox testing.  So if a function starts returning overloaded
objects instead of bare strings your tests won't notice the
difference.  This is good.

However, it does mean that functions like C<is_deeply()> cannot be used to
test the internals of string overloaded objects.  In this case I would
suggest L<Test::Deep> which contains more flexible testing functions for
complex data structures.


=item Threads

Test::More will only be aware of threads if C<use threads> has been done
I<before> Test::More is loaded.  This is ok:

    use threads;
    use Test::More;

This may cause problems:

    use Test::More
    use threads;

5.8.1 and above are supported.  Anything below that has too many bugs.

=back


=head1 HISTORY

This is a case of convergent evolution with Joshua Pritikin's L<Test>
module.  I was largely unaware of its existence when I'd first
written my own C<ok()> routines.  This module exists because I can't
figure out how to easily wedge test names into Test's interface (along
with a few other problems).

The goal here is to have a testing utility that's simple to learn,
quick to use and difficult to trip yourself up with while still
providing more flexibility than the existing Test.pm.  As such, the
names of the most common routines are kept tiny, special cases and
magic side-effects are kept to a minimum.  WYSIWYG.


=head1 SEE ALSO

=head2

=head2 ALTERNATIVES

L<Test2::Suite> is the most recent and modern set of tools for testing.

L<Test::Simple> if all this confuses you and you just want to write
some tests.  You can upgrade to Test::More later (it's forward
compatible).

L<Test::Legacy> tests written with Test.pm, the original testing
module, do not play well with other testing libraries.  Test::Legacy
emulates the Test.pm interface and does play well with others.

=head2 ADDITIONAL LIBRARIES

L<Test::Differences> for more ways to test complex data structures.
And it plays well with Test::More.

L<Test::Class> is like xUnit but more perlish.

L<Test::Deep> gives you more powerful complex data structure testing.

L<Test::Inline> shows the idea of embedded testing.

L<Mock::Quick> The ultimate mocking library. Easily spawn objects defined on
the fly. Can also override, block, or reimplement packages as needed.

L<Test::FixtureBuilder> Quickly define fixture data for unit tests.

=head2 OTHER COMPONENTS

L<Test::Harness> is the test runner and output interpreter for Perl.
It's the thing that powers C<make test> and where the C<prove> utility
comes from.

=head2 BUNDLES

L<Test::Most> Most commonly needed test functions and features.

=head1 AUTHORS

Michael G Schwern E<lt>schwern@pobox.comE<gt> with much inspiration
from Joshua Pritikin's Test module and lots of help from Barrie
Slaymaker, Tony Bowden, blackstar.co.uk, chromatic, Fergal Daly and
the perl-qa gang.

=head1 MAINTAINERS

=over 4

=item Chad Granum E<lt>exodist@cpan.orgE<gt>

=back


=head1 BUGS

See F<https://github.com/Test-More/test-more/issues> to report and view bugs.


=head1 SOURCE

The source code repository for Test::More can be found at
F<http://github.com/Test-More/test-more/>.


=head1 COPYRIGHT

Copyright 2001-2008 by Michael G Schwern E<lt>schwern@pobox.comE<gt>.

This program is free software; you can redistribute it and/or
modify it under the same terms as Perl itself.

See F<http://www.perl.com/perl/misc/Artistic.html>

=cut

1;
