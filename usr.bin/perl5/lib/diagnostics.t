#!./perl -w

BEGIN {
    chdir '..' if -d '../pod' && -d '../t';
    @INC = 'lib';
    require './t/test.pl';
    plan(31);
}

BEGIN {
    my $w;
    $SIG{__WARN__} = sub { $w = shift };
    use_ok('diagnostics');
    is $w, undef, 'no warnings when loading diagnostics.pm';
}

require base;

eval {
    'base'->import(qw(I::do::not::exist));
};

like( $@, qr/^Base class package "I::do::not::exist" is empty/,
         'diagnostics not tripped up by "use base qw(Dont::Exist)"');

open *whatever, ">", \my $warning
    or die "Couldn't redirect STDERR to var: $!";
my $old_stderr = *STDERR{IO};
*STDERR = *whatever{IO};

# Test for %.0f patterns in perldiag, added in 5.11.0
warn('gmtime(nan) too large');
like $warning, qr/\(W overflow\) You called/, '%0.f patterns';

# L<foo/bar> links
seek STDERR, 0,0;
$warning = '';
warn("accept() on closed socket spanner");
like $warning, qr/"accept" in perlfunc/, 'L<foo/bar> links';

# L<foo|bar/baz> links
seek STDERR, 0,0;
$warning = '';
warn
 'Lexing code attempted to stuff non-Latin-1 character into Latin-1 input';
like $warning, qr/lex_stuff_pvn or similar/, 'L<foo|bar/baz>';

# Multiple messages with the same description
seek STDERR, 0,0;
$warning = '';
warn 'Deep recursion on anonymous subroutine';
like $warning, qr/W recursion/,
   'Message sharing its description with the following message';
seek STDERR, 0,0;
$warning = '';
warn 'Deep recursion on subroutine "foo"';
like $warning, qr/W recursion/,
   'Message sharing its description with the preceding message';

# Periods at end of entries in perldiag.pod get matched correctly
seek STDERR, 0,0;
$warning = '';
warn "Execution of -e aborted due to compilation errors.\n";
like $warning, qr/The final summary message/, 'Periods at end of line';

# Test for %d/%u
seek STDERR, 0,0;
$warning = '';
warn "Bad arg length for us, is 4, should be 42";
like $warning, qr/In C parlance/, '%u works';

# Test for %X
seek STDERR, 0,0;
$warning = '';
warn "Unicode surrogate U+C0FFEE is illegal in UTF-8";
like $warning, qr/You had a UTF-16 surrogate/, '%X';

# Test for %p
seek STDERR, 0,0;
$warning = '';
warn "Slab leaked from cv fadedc0ffee";
like $warning, qr/bookkeeping of op trees/, '%p';

# Strip S<>
seek STDERR, 0,0;
$warning = '';
warn "syntax error";
like $warning, qr/cybernetic version of 20 questions/s, 'strip S<>';

# Errors ending with dots
seek STDERR, 0,0;
$warning = '';
warn "I had compilation errors.\n";
like $warning, qr/final summary message/, 'dotty errors';

# Multiline errors
seek STDERR, 0,0;
$warning = '';
warn "Attempt to reload weapon aborted.\nCompilation failed in require";
like $warning,
     qr/You tried to load a file.*Perl could not compile/s,
    'multiline errors';

# Multiline entry in perldiag.pod
seek STDERR, 0,0;
$warning = '';
warn "Using just the first character returned by \\N{} in character class in regex; marked by <-- HERE in m/%s/";
like $warning,
    qr/Named Unicode character escapes/s,
    'multi-line entries in perldiag.pod match';

# ; at end of entry in perldiag.pod
seek STDERR, 0,0;
$warning = '';
warn "Perl folding rules are not up-to-date for 0x0A; please use the perlbug utility to report; in regex; marked by <-- HERE in m/\ <-- HERE q/";
like $warning,
    qr/You used a regular expression with case-insensitive matching/s,
    '; works at the end of entries in perldiag.pod';

# Differences in spaces in warnings (Why not be nice and accept them?)
seek STDERR, 0,0;
$warning = '';
warn "Assignment     to both a list and a scalar\n";
like $warning,
    qr/2nd and 3rd/s,
    'spaces in warnings are matched lightly';

# Differences in spaces in warnings with a period at the end
seek STDERR, 0,0;
$warning = '';
warn "perl: warning: Setting locale failed.\n";
like $warning,
    qr/The whole warning/s,
    'spaces in warnings with periods at the end are matched lightly';

# Wrapped links
seek STDERR, 0,0;
$warning = '';
warn "Argument \"%s\" treated as 0 in increment (++)";
like $warning,
    qr/Auto-increment.*Auto-decrement/s,
    'multiline links are not truncated';

{
# Find last warning in perldiag.pod, and last items if any
    my $lw;
    my $over_level = 0;
    my $inlast;
    my $item;
    my $items_not_in_overs = 0;

    open(my $f, '<', "pod/perldiag.pod")
        or die "failed to open pod/perldiag.pod for reading: $!";

    while (<$f>) {

        # We only look for entries (=item lines) in the first level of =overs

        if ( /^=over\b/) {
            $over_level++;
        } elsif ( /^=item\s+(.*)/) {
            if ($over_level < 1) {
                $items_not_in_overs++;
            }
            elsif ($over_level == 1) {
                $lw = $1;
            }
        } elsif (/^=back\b/) {
	    $inlast = 1 if $over_level == 1;
            $over_level--;
        } elsif ($inlast) {
            # Skip headings
            next if /^=/;

            # Strip specials
            $_ =~ s/\w<(.*?)>/$1/g;

            # And whitespace
            $_ =~ s/(^\s+|\s+$)//g;

            if ($_) {
                $item = $_;

                last;
            }
        }
    }
    close($f);

    is($over_level, 0, "(sanity...) =over balanced with =back (off by $over_level)");
    is($items_not_in_overs, 0, "(sanity...) all =item lines are within =over..=back blocks");
    ok($item, "(sanity...) found an item to check with ($item)");
    seek STDERR, 0,0;
    $warning = '';
    warn $lw;
    ok($warning, '(sanity...) got a warning');
    unlike $warning,
        qr/\Q$item\E/,
        "Junk after =back doesn't show up in last warning";
}

*STDERR = $old_stderr;

# These tests use a panic under the hope that the description is not likely
# to change.
@runperl_args = (
        switches => [ '-Ilib', '-Mdiagnostics' ],
        stderr => 1,
        nolib => 1, # -I../lib would go outside the build dir
);
$subs =
 "sub foo{bar()}sub bar{baz()}sub baz{die q _panic: gremlins_}foo()";
is runperl(@runperl_args, prog => $subs),
   << 'EOT', 'internal error with backtrace';
panic: gremlins at -e line 1 (#1)
    (P) An internal error.
    
Uncaught exception from user code:
	panic: gremlins at -e line 1.
	main::baz() called at -e line 1
	main::bar() called at -e line 1
	main::foo() called at -e line 1
EOT
is runperl(@runperl_args, prog => $subs =~ s/panic\K/k/r),
   << 'EOU', 'user error with backtrace';
Uncaught exception from user code:
	panick: gremlins at -e line 1.
	main::baz() called at -e line 1
	main::bar() called at -e line 1
	main::foo() called at -e line 1
EOU
is runperl(@runperl_args, prog => 'die q _panic: gremlins_'),
   << 'EOV', 'no backtrace from top-level internal error';
panic: gremlins at -e line 1 (#1)
    (P) An internal error.
    
Uncaught exception from user code:
	panic: gremlins at -e line 1.
EOV
is runperl(@runperl_args, prog => 'die q _panick: gremlins_'),
   << 'EOW', 'no backtrace from top-level user error';
Uncaught exception from user code:
	panick: gremlins at -e line 1.
EOW
like runperl(
      @runperl_args,
      prog => $subs =~
         s[q _panic: gremlins_]
          [qq _Attempt to reload foo aborted.\\nCompilation failed in require_]r,
     ),
     qr/Uncaught exception from user code:
	Attempt to reload foo aborted\.
	Compilation failed in require at -e line \d+\.
	main::baz\(\) called at -e line \d+
	main::bar\(\) called at -e line \d+
	main::foo\(\) called at -e line \d+
/,  'backtrace from multiline error';
is runperl(@runperl_args, prog => 'BEGIN { die q _panic: gremlins_ }'),
   << 'EOX', 'BEGIN{die} does not suppress diagnostics';
panic: gremlins at -e line 1.
BEGIN failed--compilation aborted at -e line 1 (#1)
    (P) An internal error.
    
Uncaught exception from user code:
	panic: gremlins at -e line 1.
	BEGIN failed--compilation aborted at -e line 1.
EOX
