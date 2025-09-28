# RT #54452 check that filter_add does not rebless an already blessed
# given object into the callers class.

if ($] < 5.004_55) {
  print "1..0\n";
  exit 0;
}

BEGIN {
    if ($ENV{PERL_CORE}) {
        require Cwd;
        unshift @INC, Cwd::cwd();
    }
}

use strict;
use warnings;

use FindBin;
use lib "$FindBin::Bin"; # required to load filter-util.pl

require "filter-util.pl" ;

use vars qw( $Inc $Perl) ;

my $file = "bless.test" ;
my $module = "Foo";
my $bless1 = "bless1" ;

writeFile("t/Foo.pm", <<'EOM') ;
package Foo;
use strict;
use warnings;
our @ISA = ('Foo::Base');

package Foo::Base;
use Filter::Util::Call;
sub import {
  my ($class) = @_;
  my $self = bless {}, $class;
  print "before ", ref $self, "\n";
  filter_add ($self);
  print "after ", ref $self, "\n";
}
sub filter {
  my ($self) = @_;
  print "filter ", ref $self, "\n";
  return 0;
}

1;
EOM

my $fil1 = <<EOM;
use lib 't';
use Foo;
print "this is filtered out\n";
EOM

writeFile($file, $fil1);

my $a = `$Perl $Inc $file 2>&1` ;
print "1..2\n" ;

ok(1, ($? >> 8) == 0) ;
chomp $a;
ok(2, $a eq "before Foo
after Foo
filter Foo", "RT \#54452 " . $a);

unlink $file or die "Cannot remove $file: $!\n" ;
unlink "t/Foo.pm" or die "Cannot remove t/Foo.pm: $!\n" ;
