use 5.008001;

use strict;
use warnings;
use Test::More;
use Text::Balanced qw ( extract_quotelike );

our $DEBUG;
sub debug { print "\t>>>",@_ if $DEBUG }
sub esc   { my $x = shift||'<undef>'; $x =~ s/\n/\\n/gs; $x }

## no critic (BuiltinFunctions::ProhibitStringyEval)

my $cmd = "print";
my $neg = 0;
my $str;
while (defined($str = <DATA>))
{
    chomp $str;
    if ($str =~ s/\A# USING://)                 { $neg = 0; $cmd = $str; next; }
    elsif ($str =~ /\A# TH[EI]SE? SHOULD FAIL/) { $neg = 1; next; }
    elsif (!$str || $str =~ /\A#/)              { $neg = 0; next }
    my $setup_cmd = ($str =~ s/\A\{(.*)\}//) ? $1 : '';
    my $tests = 'sl';
    my $orig_str = $str;
    $str =~ s/\\n/\n/g;
    my $orig = $str;

    eval $setup_cmd if $setup_cmd ne '';
    is $@, '', 'no error';
    if($tests =~ /l/) {
        debug "\tUsing: $cmd\n";
        debug "\t   on: [" . esc($setup_cmd) . "][" . esc($str) . "]\n";
        my @res;
        eval qq{\@res = $cmd; };
        is $@, '', 'no error';
        debug "\t  got:\n" . join "", map { "\t\t\t$_: [" . esc($res[$_]) . "]\n"} (0..$#res);
        debug "\t left: [" . esc($str) . "]\n";
        debug "\t  pos: [" . esc(substr($str,pos($str))) . "...]\n";
        ($neg ? \&isnt : \&is)->(substr($str,pos($str)||0,1), ';', "$orig_str matched list");
    }

    eval $setup_cmd if $setup_cmd ne '';
    is $@, '', 'no error';
    if($tests =~ /s/) {
        $str = $orig;
        debug "\tUsing: scalar $cmd\n";
        debug "\t   on: [" . esc($str) . "]\n";
        my $var = eval $cmd;
        $var = "<undef>" unless defined $var;
        debug "\t scalar got: [" . esc($var) . "]\n";
        debug "\t scalar left: [" . esc($str) . "]\n";
        ($neg ? \&unlike : \&like)->( $str, qr/\A;/, "$orig_str matched scalar");
    }
}

# fails in Text::Balanced 1.95
$_ = qq(s{}{});
my @z = extract_quotelike();
isnt $z[0], '';

@z = extract_quotelike("<<, 1; done()\nline1\nline2\n\n and next");
like $z[1], qr/\A,/, 'implied heredoc with ,' or do {
  diag "error: '$@'\ngot: ", explain \@z;
};

done_testing;

__DATA__

# USING: extract_quotelike($str);
'';
"";
"a";
'b';
`cc`;

<<EOHERE; done();\nline1\nline2\nEOHERE\n; next;
     <<EOHERE; done();\nline1\nline2\nEOHERE\n; next;
<<"EOHERE"; done()\nline1\nline2\nEOHERE\n and next
<<`EOHERE`; done()\nline1\nline2\nEOHERE\n and next
<<'EOHERE'; done()\nline1\n'line2'\nEOHERE\n and next
<<'EOHERE;'; done()\nline1\nline2\nEOHERE;\n and next
<<"   EOHERE"; done() \nline1\nline2\n   EOHERE\nand next
<<""; done()\nline1\nline2\n\n and next
<<; done()\nline1\nline2\n\n and next
# fails in Text::Balanced 1.95
<<EOHERE;\nEOHERE\n;
# fails in Text::Balanced 1.95
<<"*";\n\n*\n;

"this is a nested $var[$x] {";
/a/gci;
m/a/gci;

q(d);
qq(e);
qx(f);
qr(g);
qw(h i j);
q{d};
qq{e};
qx{f};
qr{g};
qq{a nested { and } are okay as are () and <> pairs and escaped \}'s };
q/slash/;
q # slash #;
qr qw qx;

s/x/y/;
s/x/y/cgimsox;
s{a}{b};
s{a}\n {b};
s(a){b};
s(a)/b/;
s/'/\\'/g;
tr/x/y/;
y/x/y/;

# fails on Text-Balanced-1.95
{ $tests = 'l'; pos($str)=6 }012345<<E;\n\nE\n

# THESE SHOULD FAIL
s<$self->{pat}>{$self->{sub}}; # CAN'T HANDLE '>' in '->'
s-$self->{pap}-$self->{sub}-;  # CAN'T HANDLE '-' in '->'
<<EOHERE; done();\nline1\nline2\nEOHERE;\n; next;   # RDEL HAS NO ';'
<<'EOHERE'; done();\nline1\nline2\nEOHERE;\n; next; # RDEF HAS NO ';'
     <<    EOTHERE; done();\nline1\nline2\n    EOTHERE\n; next;  # RDEL IS "" (!)
