#!./perl

#
# test glob() in File::DosGlob
#

# Make sure it can load before other XS extensions
use File::DosGlob;

use FindBin;
use File::Spec::Functions;
BEGIN {
    chdir catdir $FindBin::Bin, (updir)x3, 't';
    @INC = '../lib';
}

use Test::More tests => 21;

# override it in main::
use File::DosGlob 'glob';

require Cwd;

my $expected;
$expected = $_ = "op/a*.t";
my @r = glob;
is ($_, $expected, 'test if $_ takes as the default');
cmp_ok(@r, '>=', 9) or diag("|@r|");

@r = <*/a*.t>;
# at least {argv,abbrev,anydbm,autoloader,append,arith,array,assignwarn,auto}.t
cmp_ok(@r, '>=', 9, 'check <*/*>') or diag("|@r|");
my $r = scalar @r;

@r = ();
while (defined($_ = <*/a*.t>)) {
    print "# $_\n";
    push @r, $_;
}
is(scalar @r, $r, 'check scalar context');

@r = ();
for (<*/a*.t>) {
    print "# $_\n";
    push @r, $_;
}
is(scalar @r, $r, 'check list context');

@r = ();
while (<*/a*.t>) {
    print "# $_\n";
    push @r, $_;
}
is(scalar @r, $r, 'implicit assign to $_ in while()');

my @s = ();
my $pat = '*/a*.t';
while (glob ($pat)) {
    print "# $_\n";
    push @s, $_;
}
is("@r", "@s", 'explicit glob() gets assign magic too');

package Foo;
use File::DosGlob 'glob';
use Test::More;
@s = ();
$pat = '*/a*.t';
while (glob($pat)) {
    print "# $_\n";
    push @s, $_;
}
is("@r", "@s", 'in a different package');

@s = ();
while (<*/a*.t>) {
    my $i = 0;
    print "# $_ <";
    push @s, $_;
    while (<*/b*.t>) {
	print " $_";
	$i++;
    }
    print " >\n";
}
is("@r", "@s", 'different glob ops maintain independent contexts');

@s = ();
eval <<'EOT';
use File::DosGlob 'GLOBAL_glob';
package Bar;
while (<*/a*.t>) {
    my $i = 0;
    print "# $_ <";
    push @s, $_;
    while (glob '*/b*.t') {
	print " $_";
	$i++;
    }
    print " >\n";
}
EOT
is("@r", "@s", 'global override');

# Test that a glob pattern containing ()'s works.
# NB. The spaces in the glob patterns need to be backslash escaped.
my $filename_containing_parens = "foo (123) bar";
SKIP: {
    skip("can't create '$filename_containing_parens': $!", 9)
	unless open my $touch, ">", $filename_containing_parens;
    close $touch;

    foreach my $pattern ("foo\\ (*", "*)\\ bar", "foo\\ (1*3)\\ bar") {
	@r = ();
	eval { @r = File::DosGlob::glob($pattern) };
	is($@, "", "eval for glob($pattern)");
	is(scalar @r, 1);
	is($r[0], $filename_containing_parens);
    }

    1 while unlink $filename_containing_parens;
}

# Test the globbing of a drive relative pattern such as "c:*.pl".
# NB. previous versions of DosGlob inserted "./ after the drive letter to
# make the expansion process work correctly. However, while it is harmless,
# there is no reason for it to be in the result.
my $cwd = Cwd::cwd();
if ($cwd =~ /^([a-zA-Z]:)/) {
    my $drive = $1;
    @r = ();
    # This assumes we're in the "t" directory.
    eval { @r = File::DosGlob::glob("${drive}io/*.t") };
    ok(@r and !grep !m|^${drive}io/[^/]*\.t$|, @r);
} else {
    pass();
}

# Test that our internal data are freed when the caller’s op tree is freed,
# even if iteration has not finished.
# Using XS::APItest is the only simple way to test this.  Since this is a
# core-only module, this should be OK.
SKIP: {
    require Config;
    skip "no XS::APItest"
     unless eval { require XS::APItest; import XS::APItest "sv_count"; 1 };
    # Use a random number of ops, so that the glob op does not reuse the
    # same address each time, giving us false passes.
    my($count,$count2);
    eval '$x+'x(1+rand() * 100) . '<*>';
    $count = sv_count();
    eval '$x+'x(1+rand() * 100) . '<*>';
    $count2 = sv_count();
    cmp_ok $count2, '<=', $count,
     'no leak when partly iterated caller is freed';
}
