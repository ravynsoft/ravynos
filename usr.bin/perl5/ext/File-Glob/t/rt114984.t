use strict;
use warnings;
use v5.16.0;
use File::Temp 'tempdir';
use File::Spec::Functions;
use Test::More;

BEGIN {
  plan skip_all => "Home-grown glob does not do character classes on $^O" if $^O eq 'VMS';
}

plan tests => 1;

my @md = (1..305);
my @mp = (1000..1205);

my $path = tempdir uc cleanup => 1;

my $md = 0;
my $mp = 0;

foreach (@md) {
    if (open(my $f, ">", catfile $path, "md_$_.dat")) {
        $md++;
        close $f;
    }
}

foreach (@mp) {
    if (open(my $f, ">", catfile $path, "mp_$_.dat")) {
        $mp++;
        close $f;
    }
}
my @b = glob(qq{$path/mp_[0123456789]*.dat $path/md_[0123456789]*.dat});
if ($md+$mp < @md+@mp) {
   warn sprintf("$0: expected to create %d files, created only %d (path $path)\n",
                @md+@mp, $md+$mp);
}
is scalar(@b), $md+$mp,
    'File::Glob extends the stack when returning a long list';
