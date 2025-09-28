#!perl -T

use strict;
use warnings;

use Config;

my $db_file;
BEGIN {
    use Config;
    foreach (qw/SDBM_File GDBM_File ODBM_File NDBM_File DB_File/) {
        if ($Config{extensions} =~ /\b$_\b/) {
            $db_file = $_;
            last;
        }
    }
}

# mini test implementation. We're going to be playing with the XS bits of
# various modules that may be used by Test::More, so it's best to avoid. Since
# XSLoader is dual life, we can't use something like perl's t/test.pl
my $planned_tests;
my $tests;
my $passed_tests;
sub ok ($;$) {
    my ($ok, $name) = @_;
    $tests++;
    $passed_tests += 1 if $ok;
    print STDOUT "not "
        if !$ok;
    print STDOUT "ok $tests";
    print STDOUT " - $name"
        if defined $name;
    print "\n";
    return $ok;
}
sub is ($$;$) {
    my ($got, $want, $name) = @_;

    my $ok
        = !defined $want && !defined $got
        || defined $want && defined $got && $got eq $want;

    defined $_ or $_ = '[undef]'
        for $got, $want;

    ok($ok, $name)
        or diag("Got: $got\nExpected: $want");

    return $ok;
}
sub can_ok ($@) {
    my ($inv, @methods) = @_;
    die "only supports one method"
        if @methods != 1;
    ok $inv->can($methods[0]), "$inv->can('$methods[0]')";
}
sub skip ($$) {
    my ($message, $count) = @_;
    die "bad skip"
        if !$count || $count =~ /[^0-9]/;
    for (1..$count) {
        $tests++;
        print STDOUT "ok $tests # skip $message\n";
    }
    $passed_tests += $count;
    no warnings 'exiting';
    last SKIP;
}
sub like ($$;$) {
    my ($got, $want_re, $name) = @_;
    if (!ref $want_re) {
        $want_re =~ m{\A/(.*)/([a-z]*)\z}
            or die "bad regex $want_re";
        $want_re = (length $2 ? "(?$2)" : '') . $1;
    }
    my $ok = $got =~ $want_re;
    ok($ok, $name)
        or diag("Got: $got\nExpected: $want_re");
    return $ok;
}
sub diag {
    my ($message) = @_;
    $message =~ s/\n?\z/\n/;
    $message =~ s/^/# /gm;
    print STDERR $message;
}
END {
    if (!defined $planned_tests) {
        print STDERR "# No plan was declared!\n";
        $? = 254;
        return;
    }

    if ($tests != $planned_tests) {
        print STDERR "# Looks like you planned $planned_tests test but ran $tests.\n";
        $? = abs($planned_tests - $tests);
    }
    elsif ($passed_tests != $tests) {
        my $failed = $tests - $passed_tests;
        print STDERR "# Looks like you failed $failed test but ran $tests.\n";
    }
}
sub plan {
    my %opts = @_;
    die "already planned"
        if defined $planned_tests;
    if (my $skip_all = $opts{skip_all}) {
        print STDOUT "1..0 # SKIP $skip_all\n";
        $planned_tests = 0;
        exit 0;
    }
    elsif ($planned_tests = $opts{tests}) {
        print STDOUT "1..$planned_tests\n";
    }
}

###

my %modules = (
    # ModuleName  => q|code to check that it was loaded|,
    'Cwd'        => q| ::can_ok( 'Cwd' => 'fastcwd'         ) |,  # 5.7 ?
    'File::Glob' => q| ::can_ok( 'File::Glob' =>                  # 5.6
                                   $] > 5.014
                                     ? 'bsd_glob' : 'doglob') |,
    $db_file     => q| ::can_ok( $db_file => 'TIEHASH'      ) |,  # 5.0
    'Socket'     => q| ::can_ok( 'Socket' => 'inet_aton'    ) |,  # 5.0
    'Time::HiRes'=> q| ::can_ok( 'Time::HiRes' => 'usleep'  ) |,  # 5.7.3
);

plan tests => keys(%modules) * 3 + 9;

use XSLoader;

# Check functions
can_ok( 'XSLoader' => 'load' );
can_ok( 'XSLoader' => 'bootstrap_inherit' );

# Check error messages
my @cases = (
    [ 'Thwack', 'package Thwack; XSLoader::load(); 1'        ],
    [ 'Zlott' , 'package Thwack; XSLoader::load("Zlott"); 1' ],
);

for my $case (@cases) {
    my ($should_load, $codestr) = @$case;
    my $diag;

    # determine the expected diagnostic
    if ($Config{usedl}) {
        if ($case->[0] eq "Thwack" and ($] == 5.008004 or $] == 5.008005)) {
            # these versions had bugs with chained C<goto &>
            $diag = "Usage: DynaLoader::bootstrap\\(module\\)";
        } else {
            # normal diagnostic for a perl with dynamic loading
            $diag = "Can't locate loadable object for module $should_load in \@INC";
        }
    } else {
        # a perl with no dynamic loading
        $diag = "Can't load module $should_load, dynamic loading not available in this perl.";
    }

    is(eval $codestr, undef, "eval '$codestr' should die");
    like($@, qr/^$diag/, "calling XSLoader::load() under a package with no XS part");
}

# Now try to load well known XS modules
my $extensions = $Config{'extensions'};
$extensions =~ s|/|::|g;

for my $module (sort keys %modules) {
    SKIP: {
        skip "$module not available", 3 if $extensions !~ /\b$module\b/;

        eval qq{ package $module; XSLoader::load('$module', "12345678"); };
        like( $@, "/^$module object version \\S+ does not match bootstrap parameter 12345678/",
                "calling XSLoader::load() with a XS module and an incorrect version" );

        eval qq{ package $module; XSLoader::load('$module'); };
        is( $@, '',  "XSLoader::load($module)");

        eval qq{ package $module; $modules{$module}; };
    }
}

SKIP: {
    skip "Needs 5.15.6", 1 unless $] > 5.0150051;
    skip "List::Util not available", 1 if $extensions !~ /\bList::Util\b/;
    eval 'package List::Util; XSLoader::load(__PACKAGE__, "version")';
    like $@, "/^Invalid version format/",
        'correct error msg for invalid versions';
}

SKIP: {
  skip "Devel::Peek not available", 1
    unless $extensions =~ /\bDevel::Peek\b/;

  # XSLoader::load() assumes it's being called from a module, so
  # pretend it is, first find where Devel/Peek.pm is
  my $peek_file = "Devel/Peek.pm";
  my $module_path;
  for my $dir (@INC) {
    if (-f "$dir/$peek_file") {
      $module_path = "$dir/Not/Devel/Peek.pm";
      last;
    }
  }

  skip "Cannot find $peek_file", 1
    unless $module_path;

  # [perl #122455]
  # die instead of falling back to DynaLoader
  no warnings 'redefine';
  local *XSLoader::bootstrap_inherit = sub { die "Fallback to DynaLoader\n" };
  ::ok( eval <<EOS, "test correct path searched for modules")
package Not::Devel::Peek;
#line 1 "$module_path"
XSLoader::load("Devel::Peek");
EOS
    or ::diag $@;
}

SKIP: {
  skip "File::Path not available", 1
    unless eval { require File::Path };
  my $name = "phooo$$";
  File::Path::mkpath("$name/auto/Foo/Bar");
  open my $fh,
    ">$name/auto/Foo/Bar/Bar.$Config::Config{'dlext'}";
  close $fh;
  my $fell_back;
  no warnings 'redefine';
  local *XSLoader::bootstrap_inherit = sub {
    $fell_back++;
    # Break out of the calling subs
    goto the_test;
  };
  eval <<END;
#line 1 $name
package Foo::Bar;
XSLoader::load("Foo::Bar");
END
 the_test:
  ok $fell_back,
    'XSLoader will not load relative paths based on (caller)[1]';
  File::Path::rmtree($name);
}
