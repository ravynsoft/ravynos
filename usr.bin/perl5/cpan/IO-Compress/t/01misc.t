BEGIN {
    if ($ENV{PERL_CORE}) {
	chdir 't' if -d 't';
	@INC = ("../lib", "lib/compress");
    }
}

use lib qw(t t/compress);
use strict;
use warnings;
use bytes;

use Test::More ;
use CompTestUtils;

BEGIN {
    # use Test::NoWarnings, if available
    my $extra = 0 ;
    $extra = 1
        if eval { require Test::NoWarnings ;  import Test::NoWarnings; 1 };

    plan tests => 163 + $extra ;

    use_ok('Scalar::Util');
    use_ok('IO::Compress::Base::Common');
}


ok gotScalarUtilXS(), "Got XS Version of Scalar::Util"
    or diag <<EOM;
You don't have the XS version of Scalar::Util
EOM

# Compress::Zlib::Common;

sub My::testParseParameters()
{
    eval { ParseParameters(1, {}, 1) ; };
    like $@, mkErr(': Expected even number of parameters, got 1'),
            "Trap odd number of params";

    eval { ParseParameters(1, {}, undef) ; };
    like $@, mkErr(': Expected even number of parameters, got 1'),
            "Trap odd number of params";

    eval { ParseParameters(1, {}, []) ; };
    like $@, mkErr(': Expected even number of parameters, got 1'),
            "Trap odd number of params";

    eval { ParseParameters(1, {'fred' => [Parse_boolean, 0]}, fred => 'joe') ; };
    like $@, mkErr("Parameter 'fred' must be an int, got 'joe'"),
            "wanted unsigned, got undef";

    eval { ParseParameters(1, {'fred' => [Parse_unsigned, 0]}, fred => undef) ; };
    like $@, mkErr("Parameter 'fred' must be an unsigned int, got 'undef'"),
            "wanted unsigned, got undef";

    eval { ParseParameters(1, {'fred' => [Parse_signed, 0]}, fred => undef) ; };
    like $@, mkErr("Parameter 'fred' must be a signed int, got 'undef'"),
            "wanted signed, got undef";

    eval { ParseParameters(1, {'fred' => [Parse_signed, 0]}, fred => 'abc') ; };
    like $@, mkErr("Parameter 'fred' must be a signed int, got 'abc'"),
            "wanted signed, got 'abc'";

    eval { ParseParameters(1, {'fred' => [Parse_code, undef]}, fred => 'abc') ; };
    like $@, mkErr("Parameter 'fred' must be a code reference, got 'abc'"),
            "wanted code, got 'abc'";


    SKIP:
    {
        use Config;

        skip 'readonly + threads', 2
            if $Config{useithreads};

        eval { ParseParameters(1, {'fred' => [Parse_writable_scalar, 0]}, fred => 'abc') ; };
        like $@, mkErr("Parameter 'fred' not writable"),
                "wanted writable, got readonly";

        skip '\\ returns mutable value in 5.19.3', 1
            if $] >= 5.019003;

        eval { ParseParameters(1, {'fred' => [Parse_writable_scalar, 0]}, fred => \'abc') ; };
        like $@, mkErr("Parameter 'fred' not writable"),
                "wanted writable, got readonly";
    }

    my @xx;
    eval { ParseParameters(1, {'fred' => [Parse_writable_scalar, 0]}, fred => \@xx) ; };
    like $@, mkErr("Parameter 'fred' not a scalar reference"),
            "wanted scalar reference";

    local *ABC;
    eval { ParseParameters(1, {'fred' => [Parse_writable_scalar, 0]}, fred => *ABC) ; };
    like $@, mkErr("Parameter 'fred' not a scalar"),
            "wanted scalar";

    eval { ParseParameters(1, {'fred' => [Parse_any, 0]}, fred => 1, fred => 2) ; };
    like $@, mkErr("Muliple instances of 'fred' found"),
        "multiple instances";

#    my $g = ParseParameters(1, {'fred' => [Parse_unsigned|Parse_multiple, 7]}, fred => 1, fred => 2) ;
#    is_deeply $g->value('fred'), [ 1, 2 ] ;
    ok 1;

    #ok 1;

    my $got = ParseParameters(1, {'fred' => [0x1000000, 0]}, fred => 'abc') ;
    is $got->getValue('fred'), "abc", "other" ;

    $got = ParseParameters(1, {'fred' => [Parse_any, undef]}, fred => undef) ;
    ok $got->parsed('fred'), "undef" ;
    ok ! defined $got->getValue('fred'), "undef" ;

    $got = ParseParameters(1, {'fred' => [Parse_string, undef]}, fred => undef) ;
    ok $got->parsed('fred'), "undef" ;
    is $got->getValue('fred'), "", "empty string" ;

    my $xx;
    $got = ParseParameters(1, {'fred' => [Parse_writable_scalar, undef]}, fred => $xx) ;

    ok $got->parsed('fred'), "parsed" ;
    my $xx_ref = $got->getValue('fred');
    $$xx_ref = 77 ;
    is $xx, 77;

    $got = ParseParameters(1, {'fred' => [Parse_writable_scalar, undef]}, fred => \$xx) ;

    ok $got->parsed('fred'), "parsed" ;
    $xx_ref = $got->getValue('fred');

    $$xx_ref = 666 ;
    is $xx, 666;

    {
        my $got1 = ParseParameters(1, {'fred' => [Parse_writable_scalar, undef]}, $got) ;
        is $got1, $got, "Same object";

        ok $got1->parsed('fred'), "parsed" ;
        $xx_ref = $got1->getValue('fred');

        $$xx_ref = 777 ;
        is $xx, 777;
    }

    for my $type (Parse_unsigned, Parse_signed, Parse_any)
    {
        my $value = 0;
        my $got1 ;
        eval { $got1 = ParseParameters(1, {'fred' => [$type, 1]}, fred => $value) } ;

        ok ! $@;
        ok $got1->parsed('fred'), "parsed ok" ;
        is $got1->getValue('fred'), 0;
    }

    {
        # setValue/getValue
        my $value = 0;
        my $got1 ;
        eval { $got1 = ParseParameters(1, {'fred' => [Parse_any, 1]}, fred => $value) } ;

        ok ! $@;
        ok $got1->parsed('fred'), "parsed ok" ;
        is $got1->getValue('fred'), 0;
        $got1->setValue('fred' => undef);
        is $got1->getValue('fred'), undef;
    }

    {
        # twice
        my $value = 0;

        my $got = IO::Compress::Base::Parameters::new();


        ok $got->parse({'fred' => [Parse_any, 1]}, fred => $value) ;

        ok $got->parsed('fred'), "parsed ok" ;
        is $got->getValue('fred'), 0;

        ok $got->parse({'fred' => [Parse_any, 1]}, fred => undef) ;
        ok $got->parsed('fred'), "parsed ok" ;
        is $got->getValue('fred'), undef;

        ok $got->parse({'fred' => [Parse_any, 1]}, fred => 7) ;
        ok $got->parsed('fred'), "parsed ok" ;
        is $got->getValue('fred'), 7;
    }
}


My::testParseParameters();


{
    title "isaFilename" ;
    ok   isaFilename("abc"), "'abc' isaFilename";

    ok ! isaFilename(undef), "undef ! isaFilename";
    ok ! isaFilename([]),    "[] ! isaFilename";
    $main::X = 1; $main::X = $main::X ;
    ok ! isaFilename(*X),    "glob ! isaFilename";
}

{
    title "whatIsInput" ;

    my $lex = LexFile->new( my $out_file );
    open FH, ">$out_file" ;
    is whatIsInput(*FH), 'handle', "Match filehandle" ;
    close FH ;

    my $stdin = '-';
    is whatIsInput($stdin),       'handle',   "Match '-' as stdin";
    #is $stdin,                    \*STDIN,    "'-' changed to *STDIN";
    #isa_ok $stdin,                'IO::File',    "'-' changed to IO::File";
    is whatIsInput("abc"),        'filename', "Match filename";
    is whatIsInput(\"abc"),       'buffer',   "Match buffer";
    is whatIsInput(sub { 1 }, 1), 'code',     "Match code";
    is whatIsInput(sub { 1 }),    ''   ,      "Don't match code";

}

{
    title "whatIsOutput" ;

    my $lex = LexFile->new( my $out_file );
    open FH, ">$out_file" ;
    is whatIsOutput(*FH), 'handle', "Match filehandle" ;
    close FH ;

    my $stdout = '-';
    is whatIsOutput($stdout),     'handle',   "Match '-' as stdout";
    #is $stdout,                   \*STDOUT,   "'-' changed to *STDOUT";
    #isa_ok $stdout,               'IO::File',    "'-' changed to IO::File";
    is whatIsOutput("abc"),        'filename', "Match filename";
    is whatIsOutput(\"abc"),       'buffer',   "Match buffer";
    is whatIsOutput(sub { 1 }, 1), 'code',     "Match code";
    is whatIsOutput(sub { 1 }),    ''   ,      "Don't match code";

}

# U64

{
    title "U64" ;

    my $x = U64->new();
    is $x->getHigh, 0, "  getHigh is 0";
    is $x->getLow, 0, "  getLow is 0";
    ok ! $x->is64bit(), " ! is64bit";

    $x = U64->new(1,2);
    is $x->getHigh, 1, "  getHigh is 1";
    is $x->getLow, 2, "  getLow is 2";
    ok $x->is64bit(), " is64bit";

    $x = U64->new(0xFFFFFFFF,2);
    is $x->getHigh, 0xFFFFFFFF, "  getHigh is 0xFFFFFFFF";
    is $x->getLow, 2, "  getLow is 2";
    ok $x->is64bit(), " is64bit";

    $x = U64->new(7, 0xFFFFFFFF);
    is $x->getHigh, 7, "  getHigh is 7";
    is $x->getLow, 0xFFFFFFFF, "  getLow is 0xFFFFFFFF";
    ok $x->is64bit(), " is64bit";

    $x = U64->new(666);
    is $x->getHigh, 0, "  getHigh is 0";
    is $x->getLow, 666, "  getLow is 666";
    ok ! $x->is64bit(), " ! is64bit";

    title "U64 - add" ;

    $x = U64->new(0, 1);
    is $x->getHigh, 0, "  getHigh is 0";
    is $x->getLow, 1, "  getLow is 1";
    ok ! $x->is64bit(), " ! is64bit";

    $x->add(1);
    is $x->getHigh, 0, "  getHigh is 0";
    is $x->getLow, 2, "  getLow is 2";
    ok ! $x->is64bit(), " ! is64bit";

    $x = U64->new(0, 0xFFFFFFFE);
    is $x->getHigh, 0, "  getHigh is 0";
    is $x->getLow, 0xFFFFFFFE, "  getLow is 0xFFFFFFFE";
    is $x->get32bit(),  0xFFFFFFFE, "  get32bit is 0xFFFFFFFE";
    is $x->get64bit(),  0xFFFFFFFE, "  get64bit is 0xFFFFFFFE";
    ok ! $x->is64bit(), " ! is64bit";

    $x->add(1);
    is $x->getHigh, 0, "  getHigh is 0";
    is $x->getLow, 0xFFFFFFFF, "  getLow is 0xFFFFFFFF";
    is $x->get32bit(),  0xFFFFFFFF, "  get32bit is 0xFFFFFFFF";
    is $x->get64bit(),  0xFFFFFFFF, "  get64bit is 0xFFFFFFFF";
    ok ! $x->is64bit(), " ! is64bit";

    $x->add(1);
    is $x->getHigh, 1, "  getHigh is 1";
    is $x->getLow, 0, "  getLow is 0";
    is $x->get32bit(),  0x0, "  get32bit is 0x0";
    is $x->get64bit(), 0xFFFFFFFF+1, "  get64bit is 0x100000000";
    ok $x->is64bit(), " is64bit";

    $x->add(1);
    is $x->getHigh, 1, "  getHigh is 1";
    is $x->getLow, 1, "  getLow is 1";
    is $x->get32bit(),  0x1, "  get32bit is 0x1";
    is $x->get64bit(),  0xFFFFFFFF+2, "  get64bit is 0x100000001";
    ok $x->is64bit(), " is64bit";

    $x->add(1);
    is $x->getHigh, 1, "  getHigh is 1";
    is $x->getLow, 2, "  getLow is 1";
    is $x->get32bit(),  0x2, "  get32bit is 0x2";
    is $x->get64bit(),  0xFFFFFFFF+3, "  get64bit is 0x100000002";
    ok $x->is64bit(), " is64bit";

    $x = U64->new(1, 0xFFFFFFFE);
    my $y = U64->new(2, 3);

    $x->add($y);
    is $x->getHigh, 4, "  getHigh is 4";
    is $x->getLow, 1, "  getLow is 1";
    ok $x->is64bit(), " is64bit";

    title "U64 - subtract" ;

    $x = U64->new(0, 1);
    is $x->getHigh, 0, "  getHigh is 0";
    is $x->getLow, 1, "  getLow is 1";
    ok ! $x->is64bit(), " ! is64bit";

    $x->subtract(1);
    is $x->getHigh, 0, "  getHigh is 0";
    is $x->getLow, 0, "  getLow is 0";
    ok ! $x->is64bit(), " ! is64bit";

    $x = U64->new(1, 0);
    is $x->getHigh, 1, "  getHigh is 1";
    is $x->getLow, 0, "  getLow is 0";
    is $x->get32bit(),  0, "  get32bit is 0xFFFFFFFE";
    is $x->get64bit(),  0xFFFFFFFF+1, "  get64bit is 0x100000000";
    ok $x->is64bit(), " is64bit";

    $x->subtract(1);
    is $x->getHigh, 0, "  getHigh is 0";
    is $x->getLow, 0xFFFFFFFF, "  getLow is 0xFFFFFFFF";
    is $x->get32bit(),  0xFFFFFFFF, "  get32bit is 0xFFFFFFFF";
    is $x->get64bit(),  0xFFFFFFFF, "  get64bit is 0xFFFFFFFF";
    ok ! $x->is64bit(), " ! is64bit";

    $x = U64->new(2, 2);
    $y = U64->new(1, 3);

    $x->subtract($y);
    is $x->getHigh, 0, "  getHigh is 0";
    is $x->getLow, 0xFFFFFFFF, "  getLow is 1";
    ok ! $x->is64bit(), " ! is64bit";

    $x = U64->new(0x01CADCE2, 0x4E815983);
    $y = U64->new(0x19DB1DE, 0xD53E8000); # NTFS to Unix time delta

    $x->subtract($y);
    is $x->getHigh, 0x2D2B03, "  getHigh is 2D2B03";
    is $x->getLow, 0x7942D983, "  getLow is 7942D983";
    ok $x->is64bit(), " is64bit";

    title "U64 - equal" ;

    $x = U64->new(0, 1);
    is $x->getHigh, 0, "  getHigh is 0";
    is $x->getLow, 1, "  getLow is 1";
    ok ! $x->is64bit(), " ! is64bit";

    $y = U64->new(0, 1);
    is $y->getHigh, 0, "  getHigh is 0";
    is $y->getLow, 1, "  getLow is 1";
    ok ! $y->is64bit(), " ! is64bit";

    my $z = U64->new(0, 2);
    is $z->getHigh, 0, "  getHigh is 0";
    is $z->getLow, 2, "  getLow is 2";
    ok ! $z->is64bit(), " ! is64bit";

    ok $x->equal($y), "  equal";
    ok !$x->equal($z), "  ! equal";

    title "U64 - clone" ;
    $x = U64->new(21, 77);
    $z =  U64::clone($x);
    is $z->getHigh, 21, "  getHigh is 21";
    is $z->getLow, 77, "  getLow is 77";

    title "U64 - cmp.gt" ;
    $x = U64->new( 1 );
    $y = U64->new( 0 );
    cmp_ok $x->cmp($y), '>', 0, "  cmp > 0";
    is $x->gt($y), 1, "  gt";
    cmp_ok $y->cmp($x), '<', 0, "  cmp < 0";

}
