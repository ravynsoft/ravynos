#!./perl 

BEGIN {
    chdir 't' if -d 't';
    @INC = '../lib';
    $ENV{PERL5LIB} = '../lib';
}

$| = 1;

print "1..28\n";

# catch "used once" warnings
my @warns;
BEGIN { $SIG{__WARN__} = sub { push @warns, @_ }; $^W = 1 };

%x = ();
$y = 3;
@z = ();
$X::x = 13;

use vars qw($p @q %r *s &t $X::p);

my $e = !(grep /^Name "X::x" used only once: possible typo/, @warns) && 'not ';
print "${e}ok 1\n";
$e = !(grep /^Name "main::x" used only once: possible typo/, @warns) && 'not ';
print "${e}ok 2\n";
$e = !(grep /^Name "main::y" used only once: possible typo/, @warns) && 'not ';
print "${e}ok 3\n";
$e = !(grep /^Name "main::z" used only once: possible typo/, @warns) && 'not ';
print "${e}ok 4\n";
($e, @warns) = @warns != 4 && 'not ';
print "${e}ok 5\n";

# this is inside eval() to avoid creation of symbol table entries and
# to avoid "used once" warnings
eval <<'EOE';
$e = ! $main::{p} && 'not ';
print "${e}ok 6\n";
$e = ! *q{ARRAY} && 'not ';
print "${e}ok 7\n";
$e = ! *r{HASH} && 'not ';
print "${e}ok 8\n";
$e = ! $main::{s} && 'not ';
print "${e}ok 9\n";
$e = ! *t{CODE} && 'not ';
print "${e}ok 10\n";
$e = defined $X::{q} && 'not ';
print "${e}ok 11\n";
$e = ! $X::{p} && 'not ';
print "${e}ok 12\n";
EOE
$e = $@ && 'not ';
print "${e}ok 13\n";

eval q{use vars qw(@X::y !abc); $e = ! *X::y{ARRAY} && 'not '};
print "${e}ok 14\n";
$e = $@ !~ /^'!abc' is not a valid variable name/ && 'not ';
print "${e}ok 15\n";

eval 'use vars qw($x[3])';
$e = $@ !~ /^Can't declare individual elements of hash or array/ && 'not ';
print "${e}ok 16\n";

{ local $^W;
  eval 'use vars qw($!)';
  ($e, @warns) = ($@ || @warns) ? 'not ' : '';
  print "${e}ok 17\n";
};

# NB the next test only works because vars.pm has already been loaded
eval 'use warnings "vars"; use vars qw($!)';
$e = ($@ || (shift(@warns)||'') !~ /^No need to declare built-in vars/)
			&& 'not ';
print "${e}ok 18\n";

no strict 'vars';
eval 'use vars qw(@x%%)';
$e = $@ && 'not ';
print "${e}ok 19\n";
$e = ! *{'x%%'}{ARRAY} && 'not ';
print "${e}ok 20\n";
eval '$u = 3; @v = (); %w = ()';
$e = $@ && 'not ';
print "${e}ok 21\n";

use strict 'vars';
eval 'use vars qw(@y%%)';
$e = $@ !~ /^'\@y%%' is not a valid variable name under strict vars/ && 'not ';
print "${e}ok 22\n";
$e = *{'y%%'}{ARRAY} && 'not ';
print "${e}ok 23\n";
eval '$u = 3; @v = (); %w = ()';
my @errs = split /\n/, $@;
$e = @errs != 3 && 'not ';
print "${e}ok 24\n";
$e = !(grep(/^Global symbol "\$u" requires explicit package name/, @errs))
			&& 'not ';
print "${e}ok 25\n";
$e = !(grep(/^Global symbol "\@v" requires explicit package name/, @errs))
			&& 'not ';
print "${e}ok 26\n";
$e = !(grep(/^Global symbol "\%w" requires explicit package name/, @errs))
			&& 'not ';
print "${e}ok 27\n";

{
    no strict;
    eval 'use strict "refs"; my $zz = "abc"; use vars qw($foo$); my $y = $$zz;';
    $e = $@ ? "" : "not ";
    print "${e}ok 28 # use vars error check modifying other strictness\n";
}
