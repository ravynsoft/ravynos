#!./perl -t

BEGIN {
    chdir 't' if -d 't';
    @INC = '../lib';
    require './test.pl';
}

use Config;

if (exists($Config{taint_support}) && !$Config{taint_support}) {
    skip_all("perl built without taint support");
}

plan tests => 13;

my $Perl = which_perl();

my $warning;
local $SIG{__WARN__} = sub { $warning = join "\n", @_; };
my $Tmsg = 'while running with -t switch';

is( ${^TAINT}, -1, '${^TAINT} == -1' );

my $out = `$Perl -le "print q(Hello)"`;
is( $out, "Hello\n",                      '`` worked' );
like( $warning, qr/^Insecure .* $Tmsg/, '    taint warn' );

{
    no warnings 'taint';
    $warning = '';
    my $out = `$Perl -le "print q(Hello)"`;
    is( $out, "Hello\n",                      '`` worked' );
    is( $warning, '',                       '   no warnings "taint"' );
}

# Get ourselves a tainted variable.
my $filename = tempfile();
$file = $0;
$file =~ s/.*/$filename/;
ok( open(FILE, ">$file"),   'open >' ) or DIE $!;
print FILE "Stuff\n";
close FILE;
like( $warning, qr/^Insecure dependency in open $Tmsg/, 'open > taint warn' );
ok( -e $file,   '   file written' );

unlink($file);
like( $warning, qr/^Insecure dependency in unlink $Tmsg/,
                                                  'unlink() taint warn' );
ok( !-e $file,  'unlink worked' );

ok( !$^W,   "-t doesn't enable regular warnings" );


mkdir('ttdir');
open(FH,'>','ttdir/ttest.pl')or DIE $!;
print FH 'return 42';
close FH or DIE $!;

SKIP: {
    ($^O eq 'MSWin32') || skip('skip tainted do test with \ separator');
    my $test = 0;
    $test =  do '.\ttdir/ttest.pl';
    is($test, 42, 'Could "do" .\ttdir/ttest.pl');
}
{
    my $test = 0;
    $test =  do './ttdir/ttest.pl';
    is($test, 42, 'Could "do" ./ttdir/ttest.pl');
}
unlink ('./ttdir/ttest.pl');
rmdir ('ttdir');
