use strict;
use warnings;
use Encode ();

use Test::More;
if (ord("A") != 65) {
    # pad_scalar() requires constant input.  To port this to EBCDIC would
    # require copy, paste,  and changing all the values for each code page.
    plan skip_all => "ASCII-centric tests";
}
else {
    plan tests => 77;
}

use XS::APItest qw( fetch_pad_names pad_scalar );

local $SIG{__WARN__} = sub { warn $_[0] unless $_[0] =~ /Wide character in print at/ };

ok defined &fetch_pad_names, "sub imported";
ok defined &pad_scalar;

my $cv = sub {
    my $test;
};

ok fetch_pad_names($cv), "Fetch working.";
is ref fetch_pad_names($cv), ref [], 'Fetch returns an arrayref';
is @{fetch_pad_names($cv)}, 1, 'Sub has one lexical.';
is fetch_pad_names($cv)->[0], '$test', "Fetching a simple scalar works.";

$cv = sub {
    use utf8;

    my $zest = 'invariant';
    my $zèst = 'latin-1';
    
    return [pad_scalar(1, "zèst"), pad_scalar(1, "z\350st"), pad_scalar(1, "z\303\250st")];
};

my $names_av    = fetch_pad_names($cv);
my $flagged     = my $unflagged = "\$z\x{c3}\x{a8}st";
Encode::_utf8_on($flagged);

general_tests( $cv->(), $names_av, {
    results => [
                { cmp => 'latin-1', msg => 'Fetches through UTF-8.' },
                { cmp => 'latin-1', msg => 'Fetches through Latin-1.' },
                { cmp => 'NOT_IN_PAD', msg => "Doesn't fetch using octets." },
               ],
    pad_size => {
                    total     => { cmp => 2, msg => 'Sub has two lexicals.' },
                    utf8      => { cmp => 2, msg => 'Sub has only UTF-8 vars.' },
                    invariant => { cmp => 0, msg => 'Sub has no invariant vars.' },
                },
    vars    => [
                { name => '$zest', msg => 'Sub has [\$zest].', type => 'ok' },
                { name =>  "\$z\x{e8}st", msg => "Sub has [\$t\x{e8}st].", type => 'ok' },
                { name => $unflagged, msg => "Sub doesn't have [$unflagged].", type => 'not ok' },
                { name => $flagged, msg => "But does have it when flagged.", type => 'ok' },
               ],
});

$cv = do {
    my $ascii = 'Defined';
    sub {
        use utf8;
        my $партнеры = $ascii;
        return [$партнеры, pad_scalar(1, "партнеры"), pad_scalar(1, "\320\277\320\260\321\200\321\202\320\275\320\265\321\200\321\213")];
    };
};

$names_av     = fetch_pad_names($cv);
my $hex_var   =  "\$\x{43f}\x{430}\x{440}\x{442}\x{43d}\x{435}\x{440}\x{44b}";
$flagged      = $unflagged = "\$\320\277\320\260\321\200\321\202\320\275\320\265\321\200\321\213";
Encode::_utf8_on($flagged);

my $russian_var = do {
    use utf8;
    '$партнеры';
};

general_tests( $cv->(), $names_av, {
    results => [
                { cmp => 'Defined', msg => 'UTF-8 fetching works.' },
                { cmp => 'Defined', msg => 'pad_scalar fetch.' },
                { cmp => 'NOT_IN_PAD', msg => "Doesn't fetch using octets." },
               ],
    pad_size => {
                    total     => { cmp => 2, msg => 'Sub has two lexicals, including those it closed over.' },
                    utf8      => { cmp => 2, msg => 'UTF-8 in the pad.' },
                    invariant => { cmp => 0, msg => '' },
                },
    vars    => [
                { name => '$ascii', msg => 'Sub has [$ascii].', type => 'ok' },
                { name => $russian_var, msg => "Sub has [$russian_var].", type => 'ok' },
                { name => $hex_var, msg => "Sub has [$hex_var].", type => 'ok' },
                { name => $unflagged, msg => "Sub doesn't have [$unflagged]", type => 'not ok' },
                { name => $flagged, msg => "But does have it when flagged.", type => 'ok' },
               ],
});

my $leon1 = "\$L\x{e9}on";
my $leon2 = my $leon3 = "\$L\x{c3}\x{a9}on";
Encode::_utf8_on($leon2);

local $@;
$cv = eval <<"END";
    sub {
        use utf8;
        my \$Leon = 'Invariant';
        my $leon1 = 'Latin-1';
        return [ \$Leon, $leon1, $leon2, pad_scalar(1, "L\x{e9}on"), pad_scalar(1, "L\x{c3}\x{a9}on")];
    };
END

my $err = $@;
ok !$err, $@;

$names_av = fetch_pad_names($cv);

general_tests( $cv->(), $names_av, {
    results => [
                { cmp => 'Invariant', msg => '' },
                { cmp => 'Latin-1', msg => "Fetched through [$leon1]" },
                { cmp => 'Latin-1', msg => "Fetched through [$leon2]" },
                { cmp => 'Latin-1', msg => 'pad_scalar fetch.' },
                { cmp => 'NOT_IN_PAD', msg => "Doesn't fetch using octets." },
               ],
    pad_size => {
                    total     => { cmp => 2, msg => 'Sub has two lexicals' },
                    utf8      => { cmp => 2, msg => 'Latin-1 got upgraded to UTF-8.' },
                    invariant => { cmp => 0, msg => '' },
                },
    vars    => [
                { name => '$Leon', msg => 'Sub has [$Leon].', type => 'ok' },
                { name => $leon1, msg => "Sub has [$leon1].", type => 'ok' },
                { name => $leon2, msg => "Sub has [$leon2].", type => 'ok' },
                { name => $leon3, msg => "Sub doesn't have [$leon3]", type => 'not ok' },
               ],
});


{
    use utf8;
    my $Cèon = 4;
    my $str1 = "\$C\x{e8}on";
    my $str2 = my $str3 = "\$C\x{c3}\x{a8}on";
    Encode::_utf8_on($str2);

    local $@;
    $cv = eval <<"END_EVAL";
        sub { [ \$Cèon, $str1, $str2 ] };
END_EVAL
    
    $err = $@;
    ok !$err;

    $names_av = fetch_pad_names($cv);

    general_tests( $cv->(), $names_av, {
        results => [ ({ SKIP => 1 }) x 3 ],
        pad_size => {
                  total => { cmp => 1, msg => 'Sub has one lexical, which it closed over.' },
                  utf8      => { cmp => 1, msg => '' },
                  invariant => { cmp => 0, msg => '' },
                    },
        vars    => [
                { name => '$Ceon', msg => "Sub doesn't have [\$Ceon].", type => 'not ok' },
                map({ { name => $_, msg => "Sub has [$_].", type => 'ok' } } $str1, $str2 ),
                { name => $str3, msg => "Sub doesn't have [$str3]", type => 'not ok' },
                   ],
    });

}

$cv = sub {
    use utf8;
    our $戦国 = 10;
    {
        no strict 'refs';
        my ($symref, $encoded_sym) = (__PACKAGE__ . "::戦国") x 2;
        utf8::encode($encoded_sym);
        return [ $戦国, ${$symref}, ${$encoded_sym} ];
    }
};

my $flagged_our = my $unflagged_our = "\$\346\210\246\345\233\275";
Encode::_utf8_on($flagged_our);

$names_av = fetch_pad_names($cv);

general_tests( $cv->(), $names_av, {
    results => [
                { cmp => '10', msg => 'Fetched UTF-8 our var.' },
                { cmp => '10', msg => "Symref fetch of an our works." },
                { cmp => undef, msg => "..and using the encoded form yields undef." },
               ],
    pad_size => {
                    total     => { cmp => 3, msg => 'Sub has three lexicals.' },
                    utf8      => { cmp => 3, msg => 'Japanese stored as UTF-8.' },
                    invariant => { cmp => 0, msg => '' },
                },
    vars    => [
                { name => "\$\x{6226}\x{56fd}", msg => "Sub has [\$\x{6226}\x{56fd}].", type => 'ok' },
                { name => $flagged_our, msg => "Sub has [$flagged_our].", type => 'ok' },
                { name => $unflagged_our, msg => "Sub doesn't have [$unflagged_our]", type => 'not ok' },
               ],
});


{

use utf8;
{
    my $test;
    BEGIN {
        $test = "t\x{c3}\x{a8}st";
        Encode::_utf8_on($test);
    }
    use constant test => $test;
}

$cv = sub {
    my $tèst = 'Good';

    return [
        $tèst,
        pad_scalar(1, "tèst"),              #"UTF-8"
        pad_scalar(1, "t\350st"),           #"Latin-1"
        pad_scalar(1, "t\x{c3}\x{a8}st"),   #"Octal"
        pad_scalar(1, test()),              #'UTF-8 enc'
        ];
};

$names_av = fetch_pad_names($cv);

general_tests( $cv->(), $names_av, {
    results => [
                { cmp => 'Good', msg => 'Fetched through Perl.' },
                { cmp => 'Good', msg => "pad_scalar: UTF-8 works." },
                { cmp => 'Good', msg => "pad_scalar: Latin-1 works." },
                { cmp => 'NOT_IN_PAD', msg => "pad_scalar: Doesn't fetch through octets." },
                { cmp => 'Good', msg => "pad_scalar: UTF-8-through-encoding works." },
               ],
    pad_size => {
                    total     => { cmp => 1, msg => 'Sub has one lexical.' },
                    utf8      => { cmp => 1, msg => '' },
                    invariant => { cmp => 0, msg => '' },
                },
    vars    => [],
});

}

$cv = do {
    use utf8;
    sub {
        my $ニコニコ = 'katakana';
        my $にこにこ = 'hiragana';

        return [
                $ニコニコ,
                $にこにこ,
                pad_scalar(1, "にこにこ"),
                pad_scalar(1, "\x{306b}\x{3053}\x{306b}\x{3053}"),
                pad_scalar(1, "\343\201\253\343\201\223\343\201\253\343\201\223"),
                pad_scalar(1, "ニコニコ"),
                pad_scalar(1, "\x{30cb}\x{30b3}\x{30cb}\x{30b3}"),
                pad_scalar(1, "\343\203\213\343\202\263\343\203\213\343\202\263"),
            ];
    }
};

$names_av = fetch_pad_names($cv);

general_tests( $cv->(), $names_av, {
    results => [
                { cmp => 'katakana', msg => '' },
                { cmp => 'hiragana', msg => '' },
                { cmp => 'hiragana', msg => '' },
                { cmp => 'hiragana', msg => '' },
                { cmp => 'NOT_IN_PAD', msg => '' },
                { cmp => 'katakana', msg => '' },
                { cmp => 'katakana', msg => '' },
                { cmp => 'NOT_IN_PAD', msg => '' },
               ],
    pad_size => {
                    total     => { cmp => 2, msg => 'Sub has two lexicals.' },
                    utf8      => { cmp => 2, msg => '' },
                    invariant => { cmp => 0, msg => '' },
                },
    vars    => [],
});

{
    {
        my $utf8_e;
        BEGIN {
            $utf8_e = "e";
            Encode::_utf8_on($utf8_e);
        }
        use constant utf8_e => $utf8_e;
    }
    my $e = 'Invariant';
    is pad_scalar(1, "e"), pad_scalar(1, utf8_e), 'Fetches the same thing, even if invariant but with differing utf8ness.';
}


sub general_tests {
    my ($results, $names_av, $tests) = @_;

    for my $i (0..$#$results) {
        next if $tests->{results}[$i]{SKIP};
        is $results->[$i], $tests->{results}[$i]{cmp}, $tests->{results}[$i]{msg};
    }

    is @$names_av, $tests->{pad_size}{total}{cmp}, $tests->{pad_size}{total}{msg};
    is grep( Encode::is_utf8($_), @$names_av),
       $tests->{pad_size}{utf8}{cmp}, $tests->{pad_size}{utf8}{msg};
    is grep( !Encode::is_utf8($_), @$names_av), $tests->{pad_size}{invariant}{cmp},
       $tests->{pad_size}{invariant}{msg};

    for my $var (@{$tests->{vars}}) {
        if ($var->{type} eq 'ok') {
            ok +(grep { $_ eq $var->{name} } @$names_av), $var->{msg};
        } else {
            ok !(grep { $_ eq $var->{name} } @$names_av), $var->{msg};
        }
    }

}
