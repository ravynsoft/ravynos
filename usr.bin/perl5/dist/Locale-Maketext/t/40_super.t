#!/usr/bin/perl -Tw

use strict;
use Test::More tests=>19;

BEGIN {
    use_ok( 'Locale::Maketext' );
}

print "#\n# Testing non-tight insertion of super-ordinate language tags...\n#\n";

my @in = grep m/\S/, split /[\n\r]/, q{
 NIX => NIX
  sv => sv
  en => en
 hai => hai

          pt-br => pt-br pt
       pt-br fr => pt-br fr pt
    pt-br fr pt => pt-br fr pt
 pt-br fr pt de => pt-br fr pt de
 de pt-br fr pt => de pt-br fr pt
    de pt-br fr => de pt-br fr pt
   hai pt-br fr => hai pt-br fr  pt

# Now test multi-part complicateds:
   pt-br-janeiro fr => pt-br-janeiro fr pt-br pt
pt-br-janeiro de fr => pt-br-janeiro de fr pt-br pt
pt-br-janeiro de pt fr => pt-br-janeiro de pt fr pt-br

ja    pt-br-janeiro fr => ja pt-br-janeiro fr pt-br pt
ja pt-br-janeiro de fr => ja pt-br-janeiro de fr pt-br pt
ja pt-br-janeiro de pt fr => ja pt-br-janeiro de pt fr pt-br

pt-br-janeiro de pt-br fr => pt-br-janeiro de pt-br fr pt
 # an odd case, since we don't filter for uniqueness in this sub

};

$Locale::Maketext::MATCH_SUPERS_TIGHTLY = 0;

foreach my $in ( @in ) {
    $in =~ s/^\s+//s;
    $in =~ s/\s+$//s;
    $in =~ s/#.+//s;
    next unless $in =~ m/\S/;

    die "What kind of line is <$in>?!"
        unless $in =~ m/^(.+)=>(.+)$/s;

    my ($i,$s) = ($1, $2);
    my @in     = ($i =~ m/(\S+)/g);
    my @should = ($s =~ m/(\S+)/g);

    my @out = Locale::Maketext->_add_supers(
        ("@in" eq 'NIX') ? () : @in
    );
    @out = 'NIX' unless @out;

    is_deeply( \@out, \@should, "Happily got [@out] from $in" );
}
