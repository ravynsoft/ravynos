use 5.008001;

use strict;
use warnings;
use Test::More;
use Text::Balanced qw ( extract_tagged gen_extract_tagged );

our $DEBUG;
sub debug { print "\t>>>",@_ if $DEBUG }

## no critic (BuiltinFunctions::ProhibitStringyEval)

my $cmd = "print";
my $neg = 0;
my $str;
while (defined($str = <DATA>))
{
    chomp $str;
    if ($str =~ s/\A# USING://) { $neg = 0; $cmd = $str; next; }
    elsif ($str =~ /\A# TH[EI]SE? SHOULD FAIL/) { $neg = 1; next; }
    elsif (!$str || $str =~ /\A#/) { $neg = 0; next }
    my $orig_str = $str;
    $str =~ s/\\n/\n/g;
    debug "\tUsing: $cmd\n";
    debug "\t   on: [$str]\n";

    my @res;
    my $var = eval "\@res = $cmd";
    is $@, '', 'no error';
    debug "\t list got: [" . join("|",map {defined $_ ? $_ : '<undef>'} @res) . "]\n";
    debug "\t list left: [$str]\n";
    ($neg ? \&isnt : \&is)->(substr($str,pos($str)||0,1), ';', "$orig_str matched list");

    pos $str = 0;
    $var = eval $cmd;
    is $@, '', 'no error';
    $var = "<undef>" unless defined $var;
    debug "\t scalar got: [$var]\n";
    debug "\t scalar left: [$str]\n";
    ($neg ? \&unlike : \&like)->( $str, qr/\A;/, "$orig_str matched scalar");
}

done_testing;

__DATA__
# USING: gen_extract_tagged("BEGIN([A-Z]+)",'END$1',"(?s).*?(?=BEGIN)")->($str);
    ignore\n this and then BEGINHERE at the ENDHERE;
    ignore\n this and then BEGINTHIS at the ENDTHIS;

# USING: extract_tagged($str,"BEGIN([A-Z]+)",'END$1',"(?s).*?(?=BEGIN)");
    ignore\n this and then BEGINHERE at the ENDHERE;
    ignore\n this and then BEGINTHIS at the ENDTHIS;

# USING: extract_tagged($str,"BEGIN([A-Z]+)",'END$1',"(?s).*?(?=BEGIN)");
    ignore\n this and then BEGINHERE at the ENDHERE;
    ignore\n this and then BEGINTHIS at the ENDTHIS;

# THIS SHOULD FAIL
    ignore\n this and then BEGINTHIS at the ENDTHAT;

# USING: extract_tagged($str,"BEGIN","END","(?s).*?(?=BEGIN)");
    ignore\n this and then BEGIN at the END;

# USING: extract_tagged($str);
    <A-1 HREF="#section2">some text</A-1>;

# USING: extract_tagged($str,qr/<[A-Z]+>/,undef, undef, {ignore=>["<BR>"]});
    <A>aaa<B>bbb<BR>ccc</B>ddd</A>;

# USING: extract_tagged($str,"BEGIN","END");
    BEGIN at the BEGIN keyword and END at the END;
    BEGIN at the beginning and end at the END;

# USING: extract_tagged($str,undef,undef,undef,{ignore=>["<[^>]*/>"]});
    <A>aaa<B>bbb<BR/>ccc</B>ddd</A>;

# USING: extract_tagged($str,";","-",undef,{reject=>[";"],fail=>"MAX"});
    ; at the ;-) keyword

# USING: extract_tagged($str,"<[A-Z]+>",undef, undef, {ignore=>["<BR>"]});
    <A>aaa<B>bbb<BR>ccc</B>ddd</A>;

# THESE SHOULD FAIL
    BEGIN at the beginning and end at the end;
    BEGIN at the BEGIN keyword and END at the end;

# TEST EXTRACTION OF TAGGED STRINGS
# USING: extract_tagged($str,"BEGIN","END",undef,{reject=>["BEGIN","END"]});
# THESE SHOULD FAIL
    BEGIN at the BEGIN keyword and END at the end;

# USING: extract_tagged($str,";","-",undef,{reject=>[";"],fail=>"PARA"});
    ; at the ;-) keyword


# USING: extract_tagged($str);
    <A>some text</A>;
    <B>some text<A>other text</A></B>;
    <A>some text<A>other text</A></A>;
    <A HREF="#section2">some text</A>;

# THESE SHOULD FAIL
    <A>some text
    <A>some text<A>other text</A>;
    <B>some text<A>other text</B>;
