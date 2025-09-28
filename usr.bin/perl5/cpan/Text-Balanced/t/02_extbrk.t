use 5.008001;

use strict;
use warnings;
use Test::More;
use Text::Balanced qw ( extract_bracketed );

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

    my $var = eval "() = $cmd";
    debug "\t list got: [$var]\n";
    debug "\t list left: [$str]\n";
    ($neg ? \&isnt : \&is)->(substr($str,pos($str)||0,1), ';', "$orig_str matched list");
    diag $@ if $@ && $DEBUG;

    pos $str = 0;
    $var = eval $cmd;
    $var = "<undef>" unless defined $var;
    debug "\t scalar got: [$var]\n";
    debug "\t scalar left: [$str]\n";
    ($neg ? \&unlike : \&like)->( $str, qr/\A;/, "$orig_str matched scalar");
    diag $@ if $@ && $DEBUG;
}

done_testing;

__DATA__

# USING: extract_bracketed($str);
{a nested { and } are okay as are () and <> pairs and escaped \}'s };
{a nested\n{ and } are okay as are\n() and <> pairs and escaped \}'s };

# USING: extract_bracketed($str,'{}');
{a nested { and } are okay as are unbalanced ( and < pairs and escaped \}'s };

# THESE SHOULD FAIL
{an unmatched nested { isn't okay, nor are ( and < };
{an unbalanced nested [ even with } and ] to match them;


# USING: extract_bracketed($str,'<"`q>');
<a q{uoted} ">" unbalanced right bracket of /(q>)/ either sort (`>>>""">>>>`) is okay >;

# USING: extract_bracketed($str,'<">');
<a quoted ">" unbalanced right bracket is okay >;

# USING: extract_bracketed($str,'<"`>');
<a quoted ">" unbalanced right bracket of either sort (`>>>""">>>>`) is okay >;

# THIS SHOULD FAIL
<a misquoted '>' unbalanced right bracket is bad >;
