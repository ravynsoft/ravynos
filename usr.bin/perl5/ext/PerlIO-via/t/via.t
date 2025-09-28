#!./perl

BEGIN {
    require Config;
    if (($Config::Config{'extensions'} !~ m!\bPerlIO/via\b!) ){
        print "1..0 # Skip -- Perl configured without PerlIO::via module\n";
        exit 0;
    }
}

use strict;
use warnings;

my $tmp = "via$$";

use Test::More tests => 26;

my $fh;
my $a = join("", map { chr } 0..255) x 10;
my $b;

BEGIN { use_ok('PerlIO::via::QuotedPrint'); }

ok( !open($fh,"<via(PerlIO::via::QuotedPrint)", $tmp), 'open QuotedPrint for input fails');
ok(  open($fh,">via(PerlIO::via::QuotedPrint)", $tmp), 'open QuotedPrint for output');
ok( (print $fh $a), "print to output file");
ok( close($fh), 'close output file');

ok( open($fh,"<via(PerlIO::via::QuotedPrint)", $tmp), 'open QuotedPrint for input');
{ local $/; $b = <$fh> }
ok( close($fh), "close input file");

is($a, $b, 'compare original data with filtered version');


{
    my $warnings = '';
    local $SIG{__WARN__} = sub { $warnings = join '', @_ };

    use warnings 'layer';

    # Find fd number we should be using
    my $fd = open($fh,'>',$tmp) && fileno($fh);
    print $fh "Hello\n";
    close($fh);

    ok( ! open($fh,">via(Unknown::Module)", $tmp), 'open via Unknown::Module will fail');
    like( $warnings, qr/^Cannot find package 'Unknown::Module'/,  'warn about unknown package' );

    # Now open normally again to see if we get right fileno
    my $fd2 = open($fh,'<',$tmp) && fileno($fh);
    is($fd2,$fd,"Wrong fd number after failed open");

    my $data = <$fh>;

    is($data,"Hello\n","File clobbered by failed open");

    close($fh);

{
package Incomplete::Module; 
}

    $warnings = '';
    no warnings 'layer';
    ok( ! open($fh,">via(Incomplete::Module)", $tmp), 'open via Incomplete::Module will fail');
    is( $warnings, "",  "don't warn about unknown package" );

    $warnings = '';
    no warnings 'layer';
    ok( ! open($fh,">via(Unknown::Module)", $tmp), 'open via Unknown::Module will fail');
    is( $warnings, "",  "don't warn about unknown package" );
}

my $obj = '';
sub Foo::PUSHED			{ $obj = shift; -1; }
sub PerlIO::via::Bar::PUSHED	{ $obj = shift; -1; }
open $fh, '<:via(Foo)', "foo";
is( $obj, 'Foo', 'search for package Foo' );
open $fh, '<:via(Bar)', "bar";
is( $obj, 'PerlIO::via::Bar', 'search for package PerlIO::via::Bar' );

{
    # [perl #131221]
    ok(open(my $fh1, ">", $tmp), "open $tmp");
    ok(binmode($fh1, ":via(XXX)"), "binmode :via(XXX) onto it");
    ok(open(my $fh2, ">&", $fh1), "dup it");
    close $fh1;
    close $fh2;

    # make sure the old workaround still works
    ok(open($fh1, ">", $tmp), "open $tmp");
    ok(binmode($fh1, ":via(YYY)"), "binmode :via(YYY) onto it");
    ok(open($fh2, ">&", $fh1), "dup it");
    print $fh2 "XZXZ";
    close $fh1;
    close $fh2;

    ok(open($fh1, "<", $tmp), "open $tmp for check");
    { local $/; $b = <$fh1> }
    close $fh1;
    is($b, "XZXZ", "check result is from non-filtering class");

    package PerlIO::via::XXX;

    sub PUSHED {
        my $class = shift;
        bless {}, $class;
    }

    sub WRITE {
        my ($self, $buffer, $handle) = @_;

        print $handle $buffer;
        return length($buffer);
    }
    package PerlIO::via::YYY;

    sub PUSHED {
        my $class = shift;
        bless {}, $class;
    }

    sub WRITE {
        my ($self, $buffer, $handle) = @_;

        $buffer =~ tr/X/Y/;
        print $handle $buffer;
        return length($buffer);
    }

    sub GETARG {
        "XXX";
    }
}

END {
    1 while unlink $tmp;
}

