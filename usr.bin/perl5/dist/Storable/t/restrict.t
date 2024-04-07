#!./perl -w
#
#  Copyright 2002, Larry Wall.
#  
#  You may redistribute only under the same terms as Perl 5, as specified
#  in the README file that comes with the distribution.
#

sub BEGIN {
    unshift @INC, 't';
    unshift @INC, 't/compat' if $] < 5.006002;
    if ($ENV{PERL_CORE}){
        require Config;
        if ($Config::Config{'extensions'} !~ /\bStorable\b/) {
            print "1..0 # Skip: Storable was not built\n";
            exit 0;
        }
    } else {
	if (!eval "require Hash::Util") {
            if ($@ =~ /Can\'t locate Hash\/Util\.pm in \@INC/s) {
                print "1..0 # Skip: No Hash::Util:\n";
                exit 0;
            } else {
                die;
            }
        }
	unshift @INC, 't';
    }
}


use Storable qw(dclone freeze thaw);
use Hash::Util qw(lock_hash unlock_value lock_keys);
use Config;
$Storable::DEBUGME = $ENV{STORABLE_DEBUGME};
use Test::More tests => (!$Storable::DEBUGME && $Config{usecperl} ? 105 : 304);

my %hash = (question => '?', answer => 42, extra => 'junk', undef => undef);
lock_hash %hash;
unlock_value %hash, 'answer';
unlock_value %hash, 'extra';
delete $hash{'extra'};

my $test;

package Restrict_Test;

sub me_second {
  return (undef, $_[0]);
}

package main;

sub freeze_thaw {
  my $temp = freeze $_[0];
  return thaw $temp;
}

sub testit {
  my $hash = shift;
  my $cloner = shift;
  my $copy = &$cloner($hash);

  my @in_keys = sort keys %$hash;
  my @out_keys = sort keys %$copy;
  is("@in_keys", "@out_keys", "keys match after deep clone");

  # $copy = $hash;	# used in initial debug of the tests

  is(Internals::SvREADONLY(%$copy), 1, "cloned hash restricted?");

  is(Internals::SvREADONLY($copy->{question}), 1,
     "key 'question' not locked in copy?");

  is(Internals::SvREADONLY($copy->{answer}), '',
     "key 'answer' not locked in copy?");

  eval { $copy->{extra} = 15 } ;
  is($@, '', "Can assign to reserved key 'extra'?");

  eval { $copy->{nono} = 7 } ;
  isnt($@, '', "Can not assign to invalid key 'nono'?");

  is(exists $copy->{undef}, 1, "key 'undef' exists");

  is($copy->{undef}, undef, "value for key 'undef' is undefined");
}

for $Storable::canonical (0, 1) {
  for my $cloner (\&dclone, \&freeze_thaw) {
    print "# \$Storable::canonical = $Storable::canonical\n";
    testit (\%hash, $cloner);
    my $object = \%hash;
    # bless {}, "Restrict_Test";

    my %hash2;
    $hash2{"k$_"} = "v$_" for 0..16;
    lock_hash %hash2;
    for (0..16) {
      unlock_value %hash2, "k$_";
      delete $hash2{"k$_"};
    }
    my $copy = &$cloner(\%hash2);

    for (0..16) {
      my $k = "k$_";
      eval { $copy->{$k} = undef } ;
      is($@, '', "Can assign to reserved key '$k'?");
    }

    my %hv;
    $hv{a} = __PACKAGE__;
    lock_keys %hv;
    my $hv2 = &$cloner(\%hv);
    ok eval { $$hv2{a} = 70 }, 'COWs do not become read-only';
  }
}

# [perl #73972]
# broken again with cperl PERL_PERTURB_KEYS_TOP.
SKIP: {
    skip "TODO restricted Storable hashes broken with PERL_PERTURB_KEYS_TOP", 1
         if !$Storable::DEBUGME && $Config{usecperl};
    for my $n (1..100) {
        my @keys = map { "FOO$_" } (1..$n);

        my $hash1 = {};
        lock_keys(%$hash1, @keys);
        my $hash2 = dclone($hash1);

        my $success;

        $success = eval { $hash2->{$_} = 'test' for @keys; 1 };
        my $err = $@;
        ok($success, "can store in all of the $n restricted slots")
            || diag("failed with $@");

        $success = !eval { $hash2->{a} = 'test'; 1 };
        ok($success, "the hash is still restricted");
    }
}
