#!./perl

BEGIN {
    chdir 't' if -d 't';
    @INC = '../lib';
    require Config; import Config;
    if ($Config{'extensions'} !~ /\bFile\/Glob\b/i) {
        print "1..0\n";
        exit 0;
    }
}

use Test::More tests => 11;

BEGIN {
    *CORE::GLOBAL::glob = sub { "Just another Perl hacker," };
}

BEGIN {
    if ("Just another Perl hacker," ne (<*>)[0]) {
        die <<EOMessage;
Your version of perl ($]) doesn't seem to allow extensions to override
the core glob operator.
EOMessage
    }
}

BEGIN {
    use_ok('File::Glob', ':globally');
}

$_ = "op/*.t";
my @r = glob;
is($_, "op/*.t");

cmp_ok(scalar @r, '>=', 3);

@r = <*/*.t>;
# at least t/global.t t/basic.t, t/taint.t
cmp_ok(scalar @r, '>=', 3, 'check if <*/*> works');
my $r = scalar @r;

@r = ();
while (defined($_ = <*/*.t>)) {
  #print "# $_\n";
  push @r, $_;
}
is(scalar @r, $r, 'check if scalar context works');

@r = ();
for (<*/*.t>) {
  #print "# $_\n";
  push @r, $_;
}
is(scalar @r, $r, 'check if list context works');

@r = ();
while (<*/*.t>) {
  #print "# $_\n";
  push @r, $_;
}
is(scalar @r, $r, 'implicit assign to $_ in while()');

my @s = ();
while (glob('*/*.t')) {
    #print "# $_\n";
    push @s, $_;
}
is("@r", "@s", 'explicit glob() gets assign magic too');

package Foo;
use File::Glob ':globally';
@s = ();
while (glob('*/*.t')) {
    #print "# $_\n";
    push @s, $_;
}
main::is("@r", "@s", 'in a different package');

# test if different glob ops maintain independent contexts
@s = ();
my $i = 0;
while (<*/*.t>) {
  #print "# $_ <";
  push @s, $_;
  while (<bas*/*.t>) {
    #print " $_";
    $i++;
  }
  #print " >\n";
}

main::is("@r", "@s", 'different glob ops maintain independent contexts');
main::isnt($i, 0, 'different glob ops maintain independent contexts');
