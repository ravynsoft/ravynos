#!./perl -w
# Test for malfunctions of utf8 cache

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
}

use strict;
use Config ();

plan(tests => 16);

SKIP: {
skip_without_dynamic_extension("Devel::Peek", 2);

my $out = runperl(stderr => 1,
		  progs => [ split /\n/, <<'EOS' ]);
    require Devel::Peek;
    $a = qq(hello \x{1234});
    for (1..2) {
        bar(substr($a, $_, 1));
    }
    sub bar {
        $_[0] = qq(\x{4321});
        Devel::Peek::Dump($_[0]);
    }
EOS

$out =~ s/^ALLOCATED at .*\n//m
    if $Config::Config{ccflags} =~ /-DDEBUG_LEAKING_SCALARS/;
like($out, qr/\ASV =/, "check we got dump output"); # [perl #121337]

my $utf8magic = qr{ ^ \s+ MAGIC \s = .* \n
                      \s+ MG_VIRTUAL \s = .* \n
                      \s+ MG_TYPE \s = \s PERL_MAGIC_utf8 .* \n
                      \s+ MG_LEN \s = .* \n }xm;

unlike($out, qr{ $utf8magic $utf8magic }x,
       "no duplicate utf8 magic");

} # SKIP

# With bad caching, this code used to go quadratic and take 10s of minutes.
# The 'test' in this case is simply that it doesn't hang.

{
    local ${^UTF8CACHE} = 1; # enable cache, disable debugging
    my $x = "\x{100}" x 1000000;
    while ($x =~ /./g) {
	my $p = pos($x);
    }
    pass("quadratic pos");
}

# Get-magic can reallocate the PV.  Check that the cache is reset in
# such cases.

# Regexp vars
"\x{100}" =~ /(.+)/;
() = substr $1, 0, 1;
"a\x{100}" =~ /(.+)/;
is ord substr($1, 1, 1), 0x100, 'get-magic resets utf8cache on match vars';

# Substr lvalues
my $x = "a\x{100}";
my $l = \substr $x, 0;
() = substr $$l, 1, 1;
substr $x, 0, 1, = "\x{100}";
is ord substr($$l, 1, 1), 0x100, 'get-magic resets utf8cache on LVALUEs';

# defelem magic
my %h;
sub {
  $_[0] = "a\x{100}";
  () = ord substr $_[0], 1, 1;
  $h{k} = "\x{100}"x2;
  is ord substr($_[0], 1, 1), 0x100,
    'get-magic resets uf8cache on defelems';
}->($h{k});


# Overloading can also reallocate the PV.

package UTF8Toggle {
    use overload '""' => 'stringify', fallback => 1;

    sub new {
	my $class = shift;
	my $value = shift;
	my $state = shift||0;
	return bless [$value, $state], $class;
    }

    sub stringify {
	my $self = shift;
	$self->[1] = ! $self->[1];
	if ($self->[1]) {
	    utf8::downgrade($self->[0]);
	} else {
	    utf8::upgrade($self->[0]);
	}
	$self->[0];
    }
}
my $u = UTF8Toggle->new(" \x{c2}7 ");

pos $u = 2;
is pos $u, 2, 'pos on overloaded utf8 toggler';
() = "$u"; # flip flag
pos $u = 2;
is pos $u, 2, 'pos on overloaded utf8 toggler (again)';

() = ord ${\substr $u, 1};
is ord ${\substr($u, 1)}, 0xc2,
    'utf8 cache + overloading does not confuse substr lvalues';
() = "$u"; # flip flag
() = ord substr $u, 1;
is ord substr($u, 1), 0xc2,
    'utf8 cache + overloading does not confuse substr lvalues (again)';

$u = UTF8Toggle->new(" \x{c2}7 ");
() = ord ${\substr $u, 2};
{ no warnings; ${\substr($u, 2, 1)} = 0; }
is $u, " \x{c2}0 ",
    'utf8 cache + overloading does not confuse substr lvalue assignment';
$u = UTF8Toggle->new(" \x{c2}7 ");
() = "$u"; # flip flag
() = ord ${\substr $u, 2};
{ no warnings; ${\substr($u, 2, 1)} = 0; }
is $u, " \x{c2}0 ",
    'utf8 cache + overload does not confuse substr lv assignment (again)';


# Typeglobs and references should not get a cache
use utf8;

#substr
my $globref = \*αabcdefg_::_;
() = substr($$globref, 2, 3);
*_abcdefgα:: = \%αabcdefg_::;
undef %αabcdefg_::;
{ no strict; () = *{"_abcdefgα::_"} }
is substr($$globref, 2, 3), "abc", 'no utf8 pos cache on globs';

my $ref = bless [], "αabcd_";
() = substr($ref, 1, 3);
bless $ref, "_abcdα";
is substr($ref, 1, 3), "abc", 'no utf8 pos cache on references';

#length
$globref = \*αabcdefg_::_;
() = "$$globref";  # turn utf8 flag on
() = length($$globref);
*_abcdefgα:: = \%αabcdefg_::;
undef %αabcdefg_::;
{ no strict; () = *{"_abcdefgα::_"} }
is length($$globref), length("$$globref"), 'no utf8 length cache on globs';

$ref = bless [], "αabcd_";
() = "$ref"; # turn utf8 flag on
() = length $ref;
bless $ref, "α";
is length $ref, length "$ref", 'no utf8 length cache on references';
