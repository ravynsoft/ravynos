use strict;
use warnings;

my $true_ref;
my $false_ref;
BEGIN {
    $true_ref = \!!1;
    $false_ref = \!!0;
}

BEGIN {
    unshift @INC, 't';
    unshift @INC, 't/compat' if $] < 5.006002;
    require Config;
    if ($ENV{PERL_CORE} and $Config::Config{'extensions'} !~ /\bStorable\b/) {
        print "1..0 # Skip: Storable was not built\n";
        exit 0;
    }
}

use Test::More tests => 12;
use Storable qw(thaw freeze);

use constant CORE_BOOLS => defined &builtin::is_bool;

{
  my $x = $true_ref;
  my $y = ${thaw freeze \$x};
  is($y, $x);
  eval {
    $$y = 2;
  };
  isnt $@, '',
    'immortal true maintained as immortal';
}

{
  my $x = $false_ref;
  my $y = ${thaw freeze \$x};
  is($y, $x);
  eval {
    $$y = 2;
  };
  isnt $@, '',
    'immortal false maintained as immortal';
}

{
  my $true = $$true_ref;
  my $x = \$true;
  my $y = ${thaw freeze \$x};
  is($$y, $$x);
  is($$y, '1');
  SKIP: {
    skip "perl $] does not support tracking boolean values", 1
      unless CORE_BOOLS;
    BEGIN { CORE_BOOLS and warnings->unimport('experimental::builtin') }
    ok builtin::is_bool($$y);
  }
  eval {
    $$y = 2;
  };
  is $@, '',
    'mortal true maintained as mortal';
}

{
  my $false = $$false_ref;
  my $x = \$false;
  my $y = ${thaw freeze \$x};
  is($$y, $$x);
  is($$y, '');
  SKIP: {
    skip "perl $] does not support tracking boolean values", 1
      unless CORE_BOOLS;
    BEGIN { CORE_BOOLS and warnings->unimport('experimental::builtin') }
    ok builtin::is_bool($$y);
  }
  eval {
    $$y = 2;
  };
  is $@, '',
    'mortal true maintained as mortal';
}
