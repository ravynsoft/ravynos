#!./perl

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc(qw '../lib ../cpan/version/lib');
}

# XXX remove this later -- dagolden, 2010-01-13
# local *STDERR = *STDOUT;

my @syntax_cases = (
    'package Foo',
    'package Bar 1.23',
    'package Baz v1.2.3',
);

my @version_cases = <DATA>;

plan tests => 7 * @syntax_cases + 7 * (grep { $_ !~ /^#/ } @version_cases)
            + 2 * 3;

use warnings qw/syntax/;
use version;

for my $string ( @syntax_cases ) {
    eval "$string";
    is( $@, '', qq/eval "$string"/ );
    eval "$string;";
    is( $@, '', qq/eval "$string;"/ );
    eval "$string ;";
    is( $@, '', qq/eval "$string ;"/ );
    eval "{$string}";
    is( $@, '', qq/eval "{$string}"/ );
    eval "{ $string }";
    is( $@, '', qq/eval "{ $string }"/ );
    eval "${string}{}";
    is( $@, '', qq/eval "${string}{}"/ );
    eval "$string {}";
    is( $@, '', qq/eval "$string {}"/ );
}

LINE:
for my $line (@version_cases) {
    chomp $line;
    # comments in data section are just diagnostics
    if ($line =~ /^#/) {
	diag $line;
	next LINE;
    }

    my ($v, $package, $quoted, $bare, $match) = split /\t+/, $line;
    my $warning = "";
    local $SIG{__WARN__} = sub { $warning .= $_[0] . "\n" };
    $match = defined $match ? $match : "";
    $match =~ s/\s*\z//; # kill trailing spaces

    # First handle the 'package NAME VERSION' case
    foreach my $suffix (";", "{}") {
	$withversion::VERSION = undef;
	if ($package eq 'fail') {
	    eval "package withversion $v$suffix";
	    like($@, qr/$match/, "package withversion $v$suffix -> syntax error ($match)");
	    ok(! version::is_strict($v), qq{... and "$v" should also fail STRICT regex});
	}
	else {
	    my $ok = eval "package withversion $v$suffix $v eq \$withversion::VERSION";
	    ok($ok, "package withversion $v$suffix")
	      or diag( $@ ? $@ : "and \$VERSION = $withversion::VERSION");
	    ok( version::is_strict($v), qq{... and "$v" should pass STRICT regex});
	}
    }

    # Now check the version->new("V") case
    my $ver = undef;
    eval qq/\$ver = version->new("$v")/;
    if ($quoted eq 'fail') {
	like($@, qr/$match/, qq{version->new("$v") -> invalid format ($match)})
          or diag( $@ ? $@ : "and \$ver = $ver" );
	ok( ! version::is_lax($v), qq{... and "$v" should fail LAX regex});
    }
    else {
	is($@, "", qq{version->new("$v")});
	ok( version::is_lax($v), qq{... and "$v" should pass LAX regex});
    }

    # Now check the version->new(V) case, unless we're skipping it
    if ( $bare eq 'na' ) {
        pass( "... skipping version->new($v)" );
	next LINE;
    }
    $ver = undef;
    eval qq/\$ver = version->new($v)/;
    if ($bare eq 'fail') {
	like($@, qr/$match/m, qq{... and unquoted version->new($v) has same error})
          or diag( $@ ? $@ : "and \$ver = $ver" );
    }
    else {
	is($@, "", qq{... and version->new($v) is ok});
    }
}

#
# Tests for #72432 - which reports a syntax error if there's a newline
# between the package name and the version.
#
# Note that we are using 'run_perl' here - there's no problem if 
# "package Foo\n1;" is evalled.
#
for my $v ("1", "1.23", "v1.2.3") {
    ok (run_perl (prog => "package Foo\n$v; print 1;"),
                          "New line between package name and version");
    ok (run_perl (prog => "package Foo\n$v { print 1; }"),
                          "New line between package name and version");
}

# The data is organized in tab delimited format with these columns:
#
# value		package		version->new	version->new	regex
# 				quoted		unquoted
#
# For each value, it is tested using eval in the following expressions
#
# 	package foo $value;			# column 2
# and
# 	my $ver = version->new("$value");	# column 3
# and
# 	my $ver = version->new($value);		# column 4
#
# The second through fourth columns can contain 'pass' or 'fail'.
#
# For any column with 'pass', the tests makes sure that no warning/error
# was thrown.  For any column with 'fail', the tests make sure that the
# error thrown matches the regex in the last column.  The unquoted column
# may also have 'na' indicating that it's pointless to test as behavior
# is subject to the perl parser before a stringifiable value is available
# to version->new
#
# If all columns are marked 'pass', the regex column is left empty.
#
# there are multiple ways that underscores can fail depending on strict
# vs lax format so these test do not distinguish between them
#
# If the DATA line begins with a # mark, it is used as a diag comment
__DATA__
1.00		pass	pass	pass
1.00001		pass	pass	pass
0.123		pass	pass	pass
12.345		pass	pass	pass
42		pass	pass	pass
0		pass	pass	pass
0.0		pass	pass	pass
v1.2.3		pass	pass	pass
v1.2.3.4	pass	pass	pass
v0.1.2		pass	pass	pass
v0.0.0		pass	pass	pass
01		fail	pass	pass	no leading zeros
01.0203		fail	pass	pass	no leading zeros
v01		fail	pass	pass	no leading zeros
v01.02.03	fail	pass	pass	no leading zeros
.1		fail	pass	pass	0 before decimal required
.1.2		fail	pass	pass	0 before decimal required
1.		fail	pass	pass	fractional part required
1.a		fail	fail	na	fractional part required
1._		fail	fail	na	fractional part required
1.02_03		fail	pass	pass	underscore
v1.2_3		fail	pass	pass	underscore
v1.02_03	fail	pass	pass	underscore
0_		fail	fail	na	underscore
1_		fail	fail	na	underscore
1_.		fail	fail	na	underscore
1.1_		fail	fail	na	underscore
1.02_03_04	fail	fail	na	underscore
1.2.3		fail	pass	pass	dotted-decimal versions must begin with 'v'
v1.2		fail	pass	pass	dotted-decimal versions require at least three parts
v0		fail	pass	pass	dotted-decimal versions require at least three parts
v1		fail	pass	pass	dotted-decimal versions require at least three parts
v.1.2.3		fail	fail	na	dotted-decimal versions require at least three parts
v		fail	fail	na	dotted-decimal versions require at least three parts
v1.2345.6	fail	pass	pass	maximum 3 digits between decimals
undef		fail	pass	pass	non-numeric data
1a		fail	fail	na	non-numeric data
1.2a3		fail	fail	na	non-numeric data
bar		fail	fail	na	non-numeric data
_		fail	fail	na	non-numeric data
