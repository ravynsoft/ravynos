use strict;
#sub I18N::LangTags::Detect::DEBUG () {10}

use Test::More tests => 19;
BEGIN {use_ok('I18N::LangTags', 'implicate_supers_strictly');}

note('Testing strict (non-tight) insertion of super-ordinate language tags');

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


foreach my $in (@in) {
  $in =~ s/^\s+//s;
  $in =~ s/\s+$//s;
  $in =~ s/#.+//s;
  next unless $in =~ m/\S/;
  
  my(@in, @should);
  {
    die "What kind of line is <$in>?!"
     unless $in =~ m/^(.+)=>(.+)$/s;
  
    my($i,$s) = ($1, $2);
    @in     = ($i =~ m/(\S+)/g);
    @should = ($s =~ m/(\S+)/g);
  }
  my @out = I18N::LangTags::implicate_supers_strictly(
    ("@in" eq 'NIX') ? () : @in
  );
  @out = 'NIX' unless @out;

  is_deeply(\@out, \@should, "implicate_supers_strictly for [$in]");
}
