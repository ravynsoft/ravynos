#!./perl
#
#  Copyright (c) 1995-2000, Raphael Manfredi
#  
#  You may redistribute only under the same terms as Perl 5, as specified
#  in the README file that comes with the distribution.
#

BEGIN {
    unshift @INC, 't';
    unshift @INC, 't/compat' if $] < 5.006002;
    require Config; import Config;
    if ($ENV{PERL_CORE} and $Config{'extensions'} !~ /\bStorable\b/) {
        print "1..0 # Skip: Storable was not built\n";
        exit 0;
    }
}

use Test::More tests => 8;

use Storable qw(freeze nfreeze thaw);

package TIED_HASH;

sub TIEHASH {
	my $self = bless {}, shift;
	return $self;
}

sub FETCH {
	my $self = shift;
	my ($key) = @_;
	$main::hash_fetch++;
	return $self->{$key};
}

sub STORE {
	my $self = shift;
	my ($key, $val) = @_;
	$self->{$key} = $val;
}

package SIMPLE;

sub make {
	my $self = bless [], shift;
	my ($x) = @_;
	$self->[0] = $x;
	return $self;
}

package ROOT;

sub make {
	my $self = bless {}, shift;
	my $h = tie %hash, TIED_HASH;
	$self->{h} = $h;
	$self->{ref} = \%hash;
	my @pool;
	for (my $i = 0; $i < 5; $i++) {
		push(@pool, SIMPLE->make($i));
	}
	$self->{obj} = \@pool;
	my @a = ('string', $h, $self);
	$self->{a} = \@a;
	$self->{num} = [1, 0, -3, -3.14159, 456, 4.5];
	$h->{key1} = 'val1';
	$h->{key2} = 'val2';
	return $self;
};

sub num { $_[0]->{num} }
sub h   { $_[0]->{h} }
sub ref { $_[0]->{ref} }
sub obj { $_[0]->{obj} }

package main;

my $is_EBCDIC = (ord('A') == 193) ? 1 : 0;
 
my $r = ROOT->make;

my $data = '';
if (!$is_EBCDIC) {			# ASCII machine
	while (<DATA>) {
		next if /^#/;
	    $data .= unpack("u", $_);
	}
} else {
	while (<DATA>) {
		next if /^#$/;		# skip comments
		next if /^#\s+/;	# skip comments
		next if /^[^#]/;	# skip uuencoding for ASCII machines
		s/^#//;				# prepare uuencoded data for EBCDIC machines
		$data .= unpack("u", $_);
	}
}

my $expected_length = $is_EBCDIC ? 217 : 278;
is(length $data, $expected_length);
  
my $y = thaw($data);
isnt($y, undef);
is(ref $y, 'ROOT');

$Storable::canonical = 1;		# Prevent "used once" warning
$Storable::canonical = 1;
# Allow for long double string conversions.
$y->{num}->[3] += 0;
$r->{num}->[3] += 0;
is(nfreeze($y), nfreeze($r));

is($y->ref->{key1}, 'val1');
is($y->ref->{key2}, 'val2');
is($hash_fetch, 2);

my $num = $r->num;
my $ok = 1;
for (my $i = 0; $i < @$num; $i++) {
	do { $ok = 0; last } unless $num->[$i] == $y->num->[$i];
}
is($ok, 1);

__END__
#
# using Storable-0.6@11, output of: print pack("u", nfreeze(ROOT->make));
# original size: 278 bytes
#
M`P,````%!`(````&"(%8"(!8"'U8"@@M,RXQ-#$U.5@)```!R%@*`S0N-5A8
M6`````-N=6T$`P````(*!'9A;#%8````!&ME>3$*!'9A;#)8````!&ME>3)B
M"51)141?2$%32%A8`````6@$`@````,*!G-T<FEN9U@$``````I8!```````
M6%A8`````6$$`@````4$`@````$(@%AB!E-)35!,15A8!`(````!"(%88@93
M24U03$586`0"`````0B"6&(&4TE-4$Q%6%@$`@````$(@UAB!E-)35!,15A8
M!`(````!"(188@9324U03$586%A8`````V]B:@0,!``````*6%A8`````W)E
(9F($4D]/5%@`
#
# using Storable-0.6@11, output of: print '#' . pack("u", nfreeze(ROOT->make));
# on OS/390 (cp 1047) original size: 217 bytes
#
#M!0,1!-G6UN,#````!00,!!$)X\G%Q&W(P>+(`P````(*!*6!D_$````$DH6H
#M\0H$I8&3\@````22A:CR`````YF%A@0"````!@B!"(`(?0H(8/-+\?3Q]?D)
#M```!R`H#]$OU`````Y6DE`0"````!001!N+)U-?3Q0(````!"(`$$@("````
#M`0B!!!("`@````$(@@02`@(````!"(,$$@("`````0B$`````Y:"D00`````
#E!`````&(!`(````#"@:BHYF)E8<$``````0$```````````!@0``
