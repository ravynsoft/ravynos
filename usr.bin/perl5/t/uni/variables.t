#!./perl

# Checks if the parser behaves correctly in edge case
# (including weird syntax errors)

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    skip_all_without_unicode_tables();
}

use 5.016;
use utf8;
use open qw( :utf8 :std );
no warnings qw(misc reserved);

plan (tests => 66880);

# ${single:colon} should not be treated as a simple variable, but as a
# block with a label inside.
{
    no strict;

    local $@;
    eval "\${\x{30cd}single:\x{30cd}colon} = 'label, not var'";
    is ${"\x{30cd}colon"}, 'label, not var',
         '${\x{30cd}single:\x{30cd}colon} should be block-label';

    local $@;
    no utf8;
    evalbytes '${single:colon} = "block/label, not var"';
    is($::colon,
         'block/label, not var',
         '...same with ${single:colon}'
        );
}

# ${yadda'etc} and ${yadda::etc} should both work under strict
{
    local $@;
    eval q<use strict; ${flark::fleem}>;
    is($@, '', q<${package::var} works>);

    no warnings qw(syntax deprecated);
    local $@;
    eval q<use strict; ${fleem'flark}>;
    is($@, '', q<...as does ${package'var}>);
}

# The first character in ${...} should respect the rules
{
   local $@;
   use utf8;
   eval '${☭asd} = 1';
   like($@, qr/\QUnrecognized character/, q(the first character in ${...} isn't special))
}

# Checking that at least some of the special variables work
for my $v (qw( ^V ; < > ( ) {^GLOBAL_PHASE} ^W _ 1 4 0 ] ! @ / \ = )) {
  SKIP: {
    local $@;
    evalbytes "\$$v;";
    is $@, '', "No syntax error for \$$v";

    local $@;
    eval "use utf8; \$$v;";
    is $@, '', "No syntax error for \$$v under 'use utf8'";
  }
}

# Checking if the Latin-1 range behaves as expected, and that the behavior is the
# same whenever under strict or not.
for ( 0x0 .. 0xff ) {
    my @warnings;
    local $SIG {__WARN__} = sub {push @warnings, @_ };
    my $ord = utf8::unicode_to_native($_);
    my $chr = chr $ord;
    my $syntax_error = 0;   # Do we expect this code point to generate a
                            # syntax error?  Assume not, for now
    my $deprecated = 0;
    my $name;

    # A different number of tests are run depending on the branches in this
    # loop iteration.  This allows us to add skips to make the reported total
    # the same for each iteration.
    my $tests = 0;
    my $max_tests = 6;

    if ($chr =~ /[[:graph:]]/a) {
        $name = "'$chr'";
        $syntax_error = 1 if $chr eq '{';
    }
    elsif ($chr =~ /[[:space:]]/a) {
        $name = sprintf "\\x%02x, an ASCII space character", $ord;
        $syntax_error = 1;
    }
    elsif ($chr =~ /[[:cntrl:]]/a) {
        $name = sprintf "\\x%02x, an ASCII control", $ord;
        $syntax_error = 1;
    }
    elsif ($chr =~ /\pC/) {
        if ($chr eq "\N{SHY}") {
            $name = sprintf "\\x%02x, SHY", $ord;
        }
        else {
            $name = sprintf "\\x%02x, a C1 control", $ord;
        }
        $syntax_error = 1;
        $deprecated = ! $syntax_error;
    }
    elsif ($chr =~ /\p{XIDStart}/) {
        $name = sprintf "\\x%02x, a non-ASCII XIDS character", $ord;
    }
    elsif ($chr =~ /\p{XPosixSpace}/) {
        $name = sprintf "\\x%02x, a non-ASCII space character", $ord;
        $syntax_error = 1;
        $deprecated = ! $syntax_error;
    }
    else {
        $name = sprintf "\\x%02x, a non-ASCII, non-XIDS graphic character", $ord;
    }
    no warnings 'closure';
    my $esc = sprintf("%X", $ord);
    utf8::downgrade($chr);
    if ($chr !~ /\p{XIDS}/u) {
        if ($syntax_error) {
            evalbytes "\$$chr";
            like($@, qr/ syntax\ error | Unrecognized\ character /x,
                     "$name as a length-1 variable generates a syntax error");
            $tests++;
            utf8::upgrade($chr);
            eval "no strict; \$$chr = 4;",
            like($@, qr/ syntax\ error | Unrecognized\ character /x,
                     "  ... and the same under 'use utf8'");
            $tests++;
        }
        elsif ($chr =~ /[[:punct:][:digit:]]/a) {
            next if ($chr eq '#' or $chr eq '*'); # RT 133583

            # Unlike other variables, we dare not try setting the length-1
            # variables that are ASCII punctuation and digits.  This is
            # because many of these variables have meaning to the system, and
            # setting them could have side effects or not work as expected
            # (And using fresh_perl() doesn't always help.) For all these we
            # just verify that they don't generate a syntax error.
            local $@;
            evalbytes "\$$chr;";
            is $@, '', "$name as a length-1 variable doesn't generate a syntax error";
            $tests++;
            utf8::upgrade($chr);
            evalbytes "no strict; use utf8; \$$chr;",
            is $@, '', "  ... and the same under 'use utf8'";
            $tests++;
        }
        else {
            is evalbytes "no strict; \$$chr = 10",
                10,
                "$name is legal as a length-1 variable";
            $tests++;
            if ($chr =~ /[[:ascii:]]/) {
                utf8::upgrade($chr);
                is evalbytes "no strict; use utf8; \$$chr = 1",
                    1,
                    "  ... and is legal under 'use utf8'";
                $tests++;
            }
            else {
                utf8::upgrade($chr);
                local $@;
                eval "no strict; use utf8; \$$chr = 1";
                like $@,
                    qr/\QUnrecognized character \x{\E\L$esc/,
                    "  ... but is illegal as a length-1 variable under 'use utf8'";
                $tests++;
            }
        }
    }
    else {
        {
            no utf8;
            local $@;
            evalbytes "no strict; \$$chr = 1";
            is($@, '', "$name under 'no utf8', 'no strict', is a valid length-1 variable");
            $tests++;

            if ($chr !~ /[[:ascii:]]/) {
                local $@;
                evalbytes "use strict; \$$chr = 1";
                is($@,
                    '',
                    "  ... and under 'no utf8' does not have to be required under strict, even though it matches XIDS"
                );
                $tests++;

                local $@;
                evalbytes "\$a$chr = 1";
                like($@,
                    qr/Unrecognized character /,
                    "  ... but under 'no utf8', it's not allowed in length-2+ variables"
                );
                $tests++;
            }
        }
        {
            use utf8;
            my $utf8 = $chr;
            utf8::upgrade($utf8);
            local $@;
            eval "no strict; \$$utf8 = 1";
            is($@, '', "  ... and under 'use utf8', 'no strict', is a valid length-1 variable");
            $tests++;

            local $@;
            eval "use strict; \$$utf8 = 1";
            if ($chr =~ /[ab]/) {   # These are special, for sort()
                is($@, '', "  ... and under 'use utf8', 'use strict',"
                    . " is a valid length-1 variable (\$a and \$b are special)");
                $tests++;
            }
            else {
                like($@,
                    qr/Global symbol "\$$utf8" requires explicit package name/,
                    "  ... and under utf8 has to be required under strict"
                );
                $tests++;
            }
        }
    }

    if (! $deprecated) {
        if ($chr =~ /[#*]/) {

            # Length-1 variables with these two characters used to be used by
            # Perl, but now a warning is generated that they're gone.
            # Ignore such warnings.
            for (my $i = @warnings - 1; $i >= 0; $i--) {
                splice @warnings, $i, 1 if $warnings[$i] =~ /is no longer supported/;
            }
        }
        my $message = "  ... and doesn't generate any warnings";
        $message = "  TODO $message" if    $ord == 0
                                        || $chr =~ /\s/a;

        if (! ok(@warnings == 0, $message)) {
            note join "\n", @warnings;
        }
        $tests++;
    }
    elsif (! @warnings) {
        fail("  ... and generates deprecation warnings (since is deprecated)");
        $tests++;
    }
    else {
        ok((scalar @warnings == grep { $_ =~ /deprecated/ } @warnings),
           "  ... and generates deprecation warnings (only)");
        $tests++;
    }

    SKIP: {
        die "Wrong max count for tests" if $tests > $max_tests;
        skip("untaken tests", $max_tests - $tests) if $max_tests > $tests;
    }
}

{
    use utf8;
    my $ret = eval "my \$c\x{327} = 100; \$c\x{327}"; # c + cedilla
    is($@, '', "ASCII character + combining character works as a variable name");
    is($ret, 100, "  ... and returns the correct value");
}

# From Tom Christiansen's 'highly illegal variable names are now accidentally legal' mail
for my $chr (
      "\N{EM DASH}", "\x{F8FF}", "\N{POUND SIGN}", "\N{SOFT HYPHEN}",
      "\N{THIN SPACE}", "\x{11_1111}", "\x{DC00}", "\N{COMBINING DIAERESIS}",
      "\N{COMBINING ENCLOSING CIRCLE BACKSLASH}",
   )
{
   no warnings 'non_unicode';
   my $esc = sprintf("%x", ord $chr);
   local $@;
   eval "\$$chr = 1; \$$chr";
   like($@,
        qr/\QUnrecognized character \x{$esc};/,
        "\\x{$esc} is illegal for a length-one identifier"
       );
}

for my $i (0x100..0xffff) {
   my $chr = chr($i);
   my $esc = sprintf("%x", $i);
   local $@;
   eval "my \$$chr = q<test>; \$$chr;";
   if ( $chr =~ /^\p{_Perl_IDStart}$/ ) {
      is($@, '', sprintf("\\x{%04x} is XIDS, works as a length-1 variable", $i));
   }
   else {
      like($@,
           qr/\QUnrecognized character \x{$esc};/,
           "\\x{$esc} isn't XIDS, illegal as a length-1 variable",
          )
   }
}

{
    # Bleadperl v5.17.9-109-g3283393 breaks ZEFRAM/Module-Runtime-0.013.tar.gz
    # https://github.com/Perl/perl5/issues/12841
    no strict;

    local $@;
    eval <<'EOP';
    q{$} =~ /(.)/;
    is($$1, $$, q{$$1 parses as ${$1}});

    $doof = "test";
    $test = "Got here";
    $::{+$$} = *doof;

    is( $$$$1, $test, q{$$$$1 parses as ${${${$1}}}} );
EOP
    is($@, '', q{$$1 parses correctly});

    for my $chr ( q{@}, "\N{U+FF10}", "\N{U+0300}" ) {
        my $esc = sprintf("\\x{%x}", ord $chr);
        local $@;
        eval <<"    EOP";
            \$$chr = q{\$};
            \$\$$chr;
    EOP

        like($@,
             qr/syntax error|Unrecognized character/,
             qq{\$\$$esc is a syntax error}
        );
    }
}

{    
    # bleadperl v5.17.9-109-g3283393 breaks JEREMY/File-Signature-1.009.tar.gz
    # https://github.com/Perl/perl5/issues/12849
    local $@;
    my $var = 10;
    eval ' ${  var  }';

    is(
        $@,
        '',
        '${  var  } works under strict'
    );

    {
        no strict;

        for my $var ( '$', "^GLOBAL_PHASE", "^V" ) {
            eval "\${ $var}";
            is($@, '', "\${ $var} works" );
            eval "\${$var }";
            is($@, '', "\${$var } works" );
            eval "\${ $var }";
            is($@, '', "\${ $var } works" );
        }
        my $var = "\7LOBAL_PHASE";
        eval "\${ $var}";
        like($@, qr/Unrecognized character \\x07/,
             "\${ $var} generates 'Unrecognized character' error" );
        eval "\${$var }";
        like($@, qr/Unrecognized character \\x07/,
             "\${$var } generates 'Unrecognized character' error" );
        eval "\${ $var }";
        like($@, qr/Unrecognized character \\x07/,
             "\${ $var } generates 'Unrecognized character' error" );
    }
}

{
    is(
        "".eval "*{\nOIN}",
        "*main::OIN",
        "Newlines at the start of an identifier should be skipped over"
    );
    
    
    SKIP: {
        skip('Is $^U on EBCDIC 1047, BC; nothing works on 0037', 1)
                                                                if $::IS_EBCDIC;
        is(
            "".eval "*{^JOIN}",
            "*main::\nOIN",
            "  ... but \$^J is still legal"
        );
    }
    
    my $ret = eval "\${\cT\n}";
    like($@, qr/\QUnrecognized character/, '${\n\cT\n} gives an error message');
}

{
    # Prior to 5.19.4, the following changed behavior depending
    # on the presence of the newline after '@{'.
    sub foo (&) { [1] }
    my %foo = (a=>2);
    my $ret = @{ foo { "a" } };
    is($ret, $foo{a}, '@{ foo { "a" } } is parsed as @foo{a}');
    
    $ret = @{
            foo { "a" }
        };
    is($ret, $foo{a}, '@{\nfoo { "a" } } is still parsed as @foo{a}');

}
