#!./perl
#
#  Copyright (c) 1995-2000, Raphael Manfredi
#  
#  You may redistribute only under the same terms as Perl 5, as specified
#  in the README file that comes with the distribution.
#

sub BEGIN {
    unshift @INC, 't';
    unshift @INC, 't/compat' if $] < 5.006002;
    require Config; import Config;
    if ($ENV{PERL_CORE} and $Config{'extensions'} !~ /\bStorable\b/) {
        print "1..0 # Skip: Storable was not built\n";
        exit 0;
    }
    require 'st-dump.pl';
}


use Storable qw(dclone);

use Test::More tests => 14;

$a = 'toto';
$b = \$a;
$c = bless {}, CLASS;
$c->{attribute} = 'attrval';
%a = ('key', 'value', 1, 0, $a, $b, 'cvar', \$c);
@a = ('first', undef, 3, -4, -3.14159, 456, 4.5,
	$b, \$a, $a, $c, \$c, \%a);

my $aref = dclone(\@a);
isnt($aref, undef);

$dumped = &dump(\@a);
isnt($dumped, undef);

$got = &dump($aref);
isnt($got, undef);

is($got, $dumped);

package FOO; @ISA = qw(Storable);

sub make {
	my $self = bless {};
	$self->{key} = \%main::a;
	return $self;
};

package main;

$foo = FOO->make;
my $r = $foo->dclone;
isnt($r, undef);

is(&dump($foo), &dump($r));

# Ensure refs to "undef" values are properly shared during cloning
my $hash;
push @{$$hash{''}}, \$$hash{a};
is($$hash{''}[0], \$$hash{a});

my $cloned = dclone(dclone($hash));
is($$cloned{''}[0], \$$cloned{a});

$$cloned{a} = "blah";
is($$cloned{''}[0], \$$cloned{a});

# [ID 20020221.007 (#8624)] SEGV in Storable with empty string scalar object
package TestString;
sub new {
    my ($type, $string) = @_;
    return bless(\$string, $type);
}
package main;
my $empty_string_obj = TestString->new('');
my $clone = dclone($empty_string_obj);
# If still here after the dclone the fix (#17543) worked.
is(ref $clone, ref $empty_string_obj);
is($$clone, $$empty_string_obj);
is($$clone, '');


SKIP: {
# Do not fail if Tie::Hash and/or Tie::StdHash is not available
    skip 'No Tie::StdHash available', 2
	unless eval { require Tie::Hash; scalar keys %Tie::StdHash:: };
    skip 'This version of perl has problems with Tie::StdHash', 2
	if $] eq "5.008";
    tie my %tie, "Tie::StdHash" or die $!;
    $tie{array} = [1,2,3,4];
    $tie{hash} = {1,2,3,4};
    my $clone_array = dclone $tie{array};
    is("@$clone_array", "@{$tie{array}}");
    my $clone_hash = dclone $tie{hash};
    is($clone_hash->{1}, $tie{hash}{1});
}
