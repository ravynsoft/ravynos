use 5.008001;

use strict;
use warnings;
use Text::Balanced qw ( gen_extract_tagged );
use Test::More;

our $DEBUG;
sub debug { print "\t>>>",@_ if $DEBUG }

## no critic (BuiltinFunctions::ProhibitStringyEval)

my $cmd = "print";
my $neg = 0;
my $str;
while (defined($str = <DATA>))
{
    chomp $str;
    my $orig_str = $str;
    $str =~ s/\\n/\n/g;
    if ($str =~ s/\A# USING://)
    {
        $neg = 0;
        eval {
                # Capture "Subroutine main::f redefined" warning
                my @warnings;
                local $SIG{__WARN__} = sub { push @warnings, shift; };
                *f = eval $str || die;
        };
        is $@, '', 'no error';
        next;
    }
    elsif ($str =~ /\A# TH[EI]SE? SHOULD FAIL/) { $neg = 1; next; }
    elsif (!$str || $str =~ /\A#/) { $neg = 0; next }
    $str =~ s/\\n/\n/g;
    debug "\tUsing: $cmd\n";
    debug "\t   on: [$str]\n";

    my @res;
    my $var = eval { @res = f($str) };
    is $@, '', 'no error';
    debug "\t list got: [" . join("|",map {defined $_ ? $_ : '<undef>'} @res) . "]\n";
    debug "\t list left: [$str]\n";
    ($neg ? \&isnt : \&is)->(substr($str,pos($str)||0,1), ';', "$orig_str matched list");

    pos $str = 0;
    $var = eval { scalar f($str) };
    is $@, '', 'no error';
    $var = "<undef>" unless defined $var;
    debug "\t scalar got: [$var]\n";
    debug "\t scalar left: [$str]\n";
    ($neg ? \&unlike : \&like)->( $str, qr/\A;/, "$orig_str matched scalar");
}

done_testing;

__DATA__

# USING: gen_extract_tagged('{','}');
    { a test };

# USING: gen_extract_tagged(qr/<[A-Z]+>/,undef, undef, {ignore=>["<BR>"]});
    <A>aaa<B>bbb<BR>ccc</B>ddd</A>;

# USING: gen_extract_tagged("BEGIN","END");
    BEGIN at the BEGIN keyword and END at the END;
    BEGIN at the beginning and end at the END;

# USING: gen_extract_tagged(undef,undef,undef,{ignore=>["<[^>]*/>"]});
    <A>aaa<B>bbb<BR/>ccc</B>ddd</A>;

# USING: gen_extract_tagged(";","-",undef,{reject=>[";"],fail=>"MAX"});
    ; at the ;-) keyword

# USING: gen_extract_tagged("<[A-Z]+>",undef, undef, {ignore=>["<BR>"]});
    <A>aaa<B>bbb<BR>ccc</B>ddd</A>;

# THESE SHOULD FAIL
    BEGIN at the beginning and end at the end;
    BEGIN at the BEGIN keyword and END at the end;

# TEST EXTRACTION OF TAGGED STRINGS
# USING: gen_extract_tagged("BEGIN","END",undef,{reject=>["BEGIN","END"]});
# THESE SHOULD FAIL
    BEGIN at the BEGIN keyword and END at the end;

# USING: gen_extract_tagged(";","-",undef,{reject=>[";"],fail=>"PARA"});
    ; at the ;-) keyword


# USING: gen_extract_tagged();
    <A>some text</A>;
    <B>some text<A>other text</A></B>;
    <A>some text<A>other text</A></A>;
    <A HREF="#section2">some text</A>;

# THESE SHOULD FAIL
    <A>some text
    <A>some text<A>other text</A>;
    <B>some text<A>other text</B>;
