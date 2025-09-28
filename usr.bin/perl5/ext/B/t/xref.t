#!./perl

BEGIN {
    unshift @INC, 't';
    require Config;
    if (($Config::Config{'extensions'} !~ /\bB\b/) ){
        print "1..0 # Skip -- Perl configured without B module\n";
        exit 0;
    }
}

use strict;
use warnings;
no warnings 'once';
use Test::More tests => 14;

# line 50
use_ok( 'B::Xref' );

my $file = 'xreftest.out';

open SAVEOUT, ">&STDOUT" or diag $!;
close STDOUT;
# line 100
our $compilesub = B::Xref::compile("-o$file");
ok( ref $compilesub eq 'CODE', "compile() returns a coderef" );
$compilesub->(); # Compile this test script
close STDOUT;
open STDOUT, ">&SAVEOUT" or diag $!;

# Now parse the output
# line 200
my ($curfile, $cursub, $curpack) = ('') x 3;
our %xreftable = ();
open XREF, '<', $file or die "# Can't open $file: $!\n";
while (<XREF>) {
    print STDERR $_ if $ENV{PERL_DEBUG};
    chomp;
    if (/^File (.*)/) {
	$curfile = $1;
    } elsif (/^  Subroutine (.*)/) {
	$cursub = $1;
    } elsif (/^    Package (.*)/) {
	$curpack = $1;
    } elsif ($curpack eq '?' && /^      (".*")  +(.*)/
	    or /^      (\S+)\s+(.*)/) {
	$xreftable{$curfile}{$cursub}{$curpack}{$1} = $2;
    }
}
close XREF;
my $thisfile = __FILE__;

ok(
    defined $xreftable{$thisfile}{'(main)'}{main}{'$compilesub'},
    '$compilesub present in main program'
);
like(
    $xreftable{$thisfile}{'(main)'}{main}{'$compilesub'},
    qr/\bi100\b/,
    '$compilesub introduced at line 100'
);
like(
    $xreftable{$thisfile}{'(main)'}{main}{'$compilesub'},
    qr/&102\b/,
    '$compilesub coderef called at line 102'
);
ok(
    defined $xreftable{$thisfile}{'(main)'}{'(lexical)'}{'$curfile'},
    '$curfile present in main program'
);
like(
    $xreftable{$thisfile}{'(main)'}{'(lexical)'}{'$curfile'},
    qr/\bi200\b/,
    '$curfile introduced at line 200'
);
ok(
    defined $xreftable{$thisfile}{'(main)'}{main}{'%xreftable'},
    '$xreftable present in main program'
);
ok(
    defined $xreftable{$thisfile}{'Testing::Xref::foo'}{main}{'%xreftable'},
    '$xreftable used in subroutine bar'
);
is(
    $xreftable{$thisfile}{'(main)'}{main}{'&use_ok'}, '&50',
    'use_ok called at line 50'
);
is(
    $xreftable{$thisfile}{'(definitions)'}{'Testing::Xref'}{'&foo'}, 's1001',
    'subroutine foo defined at line 1001'
);
is(
    $xreftable{$thisfile}{'(definitions)'}{'Testing::Xref'}{'&bar'}, 's1002',
    'subroutine bar defined at line 1002'
);
is(
    $xreftable{$thisfile}{'Testing::Xref::bar'}{'Testing::Xref'}{'&foo'},
    '&1002', 'subroutine foo called at line 1002 by bar'
);
is(
    $xreftable{$thisfile}{'Testing::Xref::foo'}{'Testing::Xref'}{'*FOO'},
    '1001', 'glob FOO used in subroutine foo'
);

END {
    1 while unlink $file;
}

# End of tests.
# Now some stuff to feed B::Xref

# line 1000
package Testing::Xref;
sub foo { print FOO %::xreftable; }
sub bar { print FOO foo; }

