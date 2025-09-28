BEGIN {
    if( $ENV{PERL_CORE} ) {
        chdir 't';
        use File::Spec;
        @INC = (File::Spec->rel2abs('../lib') );
    }
}
use strict;
use warnings;

#sub Pod::Simple::Search::DEBUG () {5};

use Pod::Simple::Search;
use Test::More;
BEGIN { plan 'no_plan' }

# print "#  Test the scanning of the whole of \@INC ...\n";

my $x = Pod::Simple::Search->new;
die "Couldn't make an object!?" unless ok defined $x;
ok $x->inc; # make sure inc=1 is the default
# print $x->_state_as_string;
#$x->verbose(12);

use Pod::Simple;
*pretty = \&Pod::Simple::BlackBox::pretty;
*pretty = \&Pod::Simple::BlackBox::pretty;  # avoid 'once' warning

my $found = 0;
$x->callback(sub {
  # print "#  ", join("  ", map "{$_}", @_), "\n";
  ++$found;
  return;
});

# print "# \@INC == @INC\n";

my $t = time();   my($name2where, $where2name) = $x->survey();
$t = time() - $t;
ok $found;

# print "# Found $found items in $t seconds!\n# See...\n";

# my $p = pretty( $where2name, $name2where )."\n";
# $p =~ s/, +/,\n/g;
# $p =~ s/^/#  /mg;
# print $p;

# print "# OK, making sure strict and strict.pm were in there...\n";
# print "# (On Debian-based distributions Pod is stripped from\n",
#       "# strict.pm, so skip these tests.)\n";
my $nopod = not exists ($name2where->{'strict'});
SKIP: {
  skip 'No Pod for strict.pm', 3 if $nopod;
  like $name2where->{'strict'}, '/strict\.(pod|pm)$/';
  ok grep( m/strict\.(pod|pm)/, keys %$where2name);

  ok my $strictpath = $name2where->{'strict'}, 'Should have strict path';
  my @x = ($x->find('strict')||'(nil)', $strictpath);
#  print "# Comparing \"$x[0]\" to \"$x[1]\"\n";
  for(@x) { s{[/\\]}{/}g; }
#  print "#        => \"$x[0]\" to \"$x[1]\"\n";
  is $x[0], $x[1], " find('strict') should match survey's name2where{strict}";
}

# print "# Test again on a module we know is present, in case the
# strict.pm tests were skipped...\n";

# Search for all files in $name2where.
while (my ($testmod, $testpath) = each %{ $name2where }) {
  unless ( $testmod ) {
    fail;  # no 'thatpath/<name>.pm' means can't test find()
    next;
  }
  my @x = ($x->find($testmod)||'(nil)', $testpath);
  # print "# Comparing \"$x[0]\" to \"$x[1]\"\n";
  my $result = File::Spec->rel2abs($x[0]);
  # print "#        => \"$result\" to \"$x[1]\"\n";
  if ($result ne $x[1]) {
    TODO: {
      local $TODO = 'unstable Pod::Simple::Search';
      is( $result, $x[1],
          " find('$testmod') should match survey's name2where{$testmod}");
    }
  } else {
    is( $result, $x[1],
      " find('$testmod') should match survey's name2where{$testmod}");
  }
}

pass;
# print "# Byebye from ", __FILE__, "\n";
# print "# @INC\n";
__END__

