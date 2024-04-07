#!./perl

# Tests for labels in UTF-8

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
    skip_all_without_unicode_tables();
}

use utf8;
use open qw( :utf8 :std );
use warnings;
use feature qw 'unicode_strings evalbytes';

use charnames qw( :full );

plan(10);

ＬＡＢＥＬ: {
    pass("Sanity check, UTF-8 labels don't throw a syntax error.");
}


SKIP: {
    skip_if_miniperl("no dynamic loading, no Encode", 2);
    no warnings 'exiting';
    require Encode;

    my $prog = 'last ＬＯＯＰ;';

    ＬＯＯＰ: {
        eval $prog;
    }
    is $@, '', "last with a UTF-8 label works,";

    ＬＯＯＰ: {
        Encode::_utf8_off($prog);
        evalbytes $prog;
        like $@, qr/^Unrecognized character/, "..but turn off the UTF-8 flag and it explodes";
    }
}

{
    no warnings 'exiting';

    eval "last Ｅ";
    like $@, qr/Label not found for "last Ｅ" at/u, "last's error is UTF-8 clean";
    
    eval "redo Ｅ";
    like $@, qr/Label not found for "redo Ｅ" at/u, "redo's error is UTF-8 clean";
    
    eval "next Ｅ";
    like $@, qr/Label not found for "next Ｅ" at/u, "next's error is UTF-8 clean";
}

my $d = 2;
LÁBEL: {
    my $e = $@;
    my $prog = "redo L\N{LATIN CAPITAL LETTER A WITH ACUTE}BEL";

    if ($d == 1) {
        is $e, '', "redo UTF8 works";
        utf8::downgrade($prog);
    }
    if ($d--) {
        use feature 'unicode_eval';
        no warnings 'exiting';
        eval $prog;
    }
}

like $@, qr/Unrecognized character/, "redo to downgradeable labels";
is $d, 0, "Latin-1 labels are reachable";

{
    no warnings;
    goto ここ;
    
    if (undef) {
        ここ: {
            pass("goto UTF-8 LABEL works.");
        }
    }
}
