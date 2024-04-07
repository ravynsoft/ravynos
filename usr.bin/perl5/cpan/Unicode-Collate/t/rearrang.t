
BEGIN {
    if ($ENV{PERL_CORE}) {
	chdir('t') if -d 't';
	@INC = $^O eq 'MacOS' ? qw(::lib) : qw(../lib);
    }
}

use strict;
use warnings;
BEGIN { $| = 1; print "1..23\n"; }
my $count = 0;
sub ok ($;$) {
    my $p = my $r = shift;
    if (@_) {
	my $x = shift;
	$p = !defined $x ? !defined $r : !defined $r ? 0 : $r eq $x;
    }
    print $p ? "ok" : "not ok", ' ', ++$count, "\n";
}

use Unicode::Collate;

ok(1);

sub _pack_U   { Unicode::Collate::pack_U(@_) }
sub _unpack_U { Unicode::Collate::unpack_U(@_) }

#########################

my $Collator = Unicode::Collate->new(
  table => 'keys.txt',
  normalization => undef,
  UCA_Version => 9,
);

# rearrange : 0x0E40..0x0E44, 0x0EC0..0x0EC4 (default)

##### 2..9

my %old_rearrange = $Collator->change(rearrange => undef);

ok($Collator->gt( "\x{E41}A",  "\x{E40}B"));
ok($Collator->gt("A\x{E41}A", "A\x{E40}B"));

$Collator->change(rearrange => [ 0x61 ]);

# 0x61 FOR 'a' SHOULD BE A UNICODE CODE POINT, NOT A NATIVE CODE POINT.

ok($Collator->gt("ab", "AB")); # as 'ba' > 'AB'

$Collator->change(%old_rearrange);

ok($Collator->lt("ab", "AB"));
ok($Collator->lt( "\x{E40}",   "\x{E41}"));
ok($Collator->lt( "\x{E40}A",  "\x{E41}B"));
ok($Collator->lt( "\x{E41}A",  "\x{E40}B"));
ok($Collator->lt("A\x{E41}A", "A\x{E40}B"));

##### 10..13

my $all_undef_8 = Unicode::Collate->new(
  table => undef,
  normalization => undef,
  overrideCJK => undef,
  overrideHangul => undef,
  UCA_Version => 8,
);

ok($all_undef_8->lt( "\x{E40}",   "\x{E41}"));
ok($all_undef_8->lt( "\x{E40}A",  "\x{E41}B"));
ok($all_undef_8->lt( "\x{E41}A",  "\x{E40}B"));
ok($all_undef_8->lt("A\x{E41}A", "A\x{E40}B"));

##### 14..18

my $no_rearrange = Unicode::Collate->new(
  table => undef,
  normalization => undef,
  rearrange => [],
  UCA_Version => 9,
);

ok($no_rearrange->lt("A", "B"));
ok($no_rearrange->lt( "\x{E40}",   "\x{E41}"));
ok($no_rearrange->lt( "\x{E40}A",  "\x{E41}B"));
ok($no_rearrange->gt( "\x{E41}A",  "\x{E40}B"));
ok($no_rearrange->gt("A\x{E41}A", "A\x{E40}B"));

##### 19..23

my $undef_rearrange = Unicode::Collate->new(
  table => undef,
  normalization => undef,
  rearrange => undef,
  UCA_Version => 9,
);

ok($undef_rearrange->lt("A", "B"));
ok($undef_rearrange->lt( "\x{E40}",   "\x{E41}"));
ok($undef_rearrange->lt( "\x{E40}A",  "\x{E41}B"));
ok($undef_rearrange->gt( "\x{E41}A",  "\x{E40}B"));
ok($undef_rearrange->gt("A\x{E41}A", "A\x{E40}B"));

