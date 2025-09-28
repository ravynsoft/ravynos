
use strict;
use warnings;

BEGIN {
    if ($ENV{PERL_CORE}) {
        chdir 't' if -d 't';
        @INC = ("../lib", "lib/compress");
    }
}

use lib qw(t t/compress);


use Test::More ;
use CompTestUtils;

BEGIN {
    # use Test::NoWarnings, if available
    my $extra = 0 ;
    $extra = 1
        if eval { require Test::NoWarnings ;  import Test::NoWarnings; 1 };

    plan tests => 21 + $extra ;

    use_ok('IO::Compress::Zip', qw(zip $ZipError)) ;

    use_ok('IO::Uncompress::Unzip', qw($UnzipError)) ;
    use_ok('IO::Uncompress::AnyUncompress', qw($AnyUncompressError)) ;

}

    my @buffers ;
    push @buffers, <<EOM ;
hello world
this is a test
some more stuff on this line
ad finally...
EOM

    push @buffers, <<EOM ;
some more stuff
line 2
EOM

    push @buffers, <<EOM ;
even more stuff
EOM


my $name = "n1";
my $lex = LexFile->new( my $zipfile );

my $x = IO::Compress::Zip->new($zipfile, Name => $name++, AutoClose => 1);
isa_ok $x, 'IO::Compress::Zip', '  $x' ;


foreach my $buffer (@buffers) {
    ok $x->write($buffer), "    Write OK" ;
    # this will add an extra "empty" stream
    ok $x->newStream(Name => $name ++), "    newStream OK" ;
}
ok $x->close, "  Close ok" ;

push @buffers, undef;

{
    open F, ">>$zipfile";
    print F "trailing";
    close F;
}

my $u = IO::Uncompress::Unzip->new( $zipfile, Transparent => 1, MultiStream => 0 )
    or die "Cannot open $zipfile: $UnzipError";

my @names ;
my $status;
my $expname = "n1";
my $ix = 0;

for my $ix (1 .. 4)
{
    local $/ ;

    my $n = $u->getHeaderInfo()->{Name};
    is $n, $expname , "name is $expname";
    is <$u>, $buffers[$ix-1], "payload ok";
    ++ $expname;

    $status = $u->nextStream()
}

{
    local $/ ;

    my $n = $u->getHeaderInfo()->{Name};
    is $n, undef , "name is undef";
    is <$u>, "trailing", "payload ok";
}

die "Error processing $zipfile: $!\n"
    if $status < 0 ;